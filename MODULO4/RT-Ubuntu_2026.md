# Tutorial en Clase
## Implementación y Verificación de un Sistema Operativo de Tiempo Real (Real-Time Ubuntu)

## Recursos necesarios

### Hardware

- Una Raspberry Pi 4 (preferiblemente con 4 GB de RAM o más).
- Fuente de alimentación apropiada para Raspberry Pi 4.
- Tarjeta microSD de al menos 32 GB.
- Computador para preparar la tarjeta y acceder a la Raspberry Pi.
- Conexión a Internet.
- Monitor, teclado y mouse, o acceso remoto mediante SSH.

### Software

- Ubuntu Desktop 24.04 LTS ó 26.04 LTS para Raspberry Pi 4.
- Una cuenta Ubuntu One.
- Una suscripción Ubuntu Pro (la suscripción personal gratuita es suficiente).
- `rt-tests`.
- `gnuplot`.
- Compilador GCC.
- Herramientas de desarrollo de Linux.

---

## PARTE I. Preparación del entorno y mediciones con el kernel genérico

## 1. Registro e inicio de sesión en Ubuntu One

**Ubuntu One** proporciona el servicio de identificación utilizado por diferentes servicios de la empresa **Canonical** (desarrolladora de **Ubuntu**), entre ellos **Ubuntu Pro**.

### 1.1. Crear una cuenta

1. En un navegador web de su preferencia, acceda a:

   https://login.ubuntu.com

2. Seleccionar la opción para crear una cuenta.
3. Introducir una dirección de correo electrónico y los datos solicitados.
4. Completar el procedimiento de verificación de la cuenta.
5. Inicie sesión.

Si ya se dispone de una cuenta Ubuntu One, simplemente realizar el inicio de sesión.


## 2. Instalación de rt-tests

`rt-tests` es un conjunto de herramientas destinadas a evaluar diferentes características temporales de sistemas Linux.

Entre sus herramientas se encuentra `cyclictest`, que permite medir las desviaciones temporales entre los instantes en que una tarea periódica debería ejecutarse y los instantes en que realmente lo hace.

La documentación oficial de Ubuntu se encuentra en:

https://ubuntu.com/real-time/docs/how-to/measure-maximum-latency/

### 2.1. Actualización de repositorios

Abra una terminal en Ubuntu y ejecute:

```bash
sudo apt update
```

### 2.2. Instale rt-tests

```bash
sudo apt install rt-tests
```

Verifique la instalación:

```bash
cyclictest --help
```

Examine las opciones disponibles, prestando especial atención a:

- `--priority`: prioridad de ejecución.
- `--interval`: intervalo entre activaciones.
- `--duration`: duración de la prueba.
- `--loops`: número de iteraciones.
- `--histogram`: generación de histogramas.
- `--mlockall`: bloqueo de las páginas de memoria utilizadas por el proceso.
- `--smp`: ejecución de pruebas en varios procesadores.

### 2.3. Instalación de gnuplot

Para representar gráficamente los histogramas se utilizará GNUPlot.

```bash
sudo apt install gnuplot
```

Verificar la instalación:

```bash
gnuplot --version
```

## 3. Primera medición: cyclictest con el kernel genérico

Antes de configurar **Real-Time Ubuntu**, se establecerá una línea base de referencia.

El propósito de esta actividad es observar el comportamiento temporal del kernel genérico de Linux y reconocer las desviaciones que pueden producirse en la ejecución de tareas periódicas.

### 3.1. Verificación del kernel actualmente instalado

Ejecutar:

```bash
uname -r
```

También se puede ejecutar:

```bash
uname -a
```

En este punto se debe identificar que el sistema está utilizando el *kernel genérico* de Ubuntu para Raspberry Pi.

### 3.2. Ejecutar cyclictest

Ejecutar el siguiente comando:

```bash
sudo cyclictest \
    --mlockall \
    --smp \
    --priority=80 \
    --interval=200 \
    --distance=0
```

La prueba ejecuta tareas periódicas con un intervalo de 200 microsegundos y prioridad 80.

El parámetro `--smp` permite realizar mediciones en los diferentes procesadores disponibles.

La prueba continúa ejecutándose hasta que se interrumpe mediante `Ctrl+C`.

### 3.3. Interpretación de los resultados

La salida presenta información individual para cada procesador:

```text
T: 0 (PID) P:80 I:200 C:... Min: 1 Act: 4 Avg: 3 Max: 129
T: 1 (PID) P:80 I:200 C:... Min: 1 Act: 3 Avg: 3 Max: 99
```

Los campos más importantes son:

| Campo | Descripción |
|---|---|
| T | Identificador de la tarea de medición. |
| P | Prioridad de ejecución. |
| I | Intervalo de activación, en microsegundos. |
| C | Número de iteraciones realizadas. |
| Min | Menor latencia observada. |
| Act | Latencia de la última activación. |
| Avg | Latencia promedio. |
| Max | Mayor latencia observada. |

La latencia representa la diferencia entre el instante programado para despertar la tarea y el instante en que efectivamente comienza su ejecución.

Por ejemplo, si una tarea debía activarse en el instante 1000 μs y comienza a ejecutarse en 1007 μs, su latencia es de 7 μs.

**Interpretación:** el valor `Max` es especialmente importante porque permite identificar el peor retraso observado durante la prueba. Una latencia promedio pequeña no garantiza que las activaciones individuales se encuentren dentro de un límite temporal.

### 3.4. Limitación de la duración de las pruebas

Para realizar experimentos reproducibles se puede establecer un número fijo de iteraciones:

```bash
sudo cyclictest \
    --mlockall \
    --smp \
    --priority=80 \
    --interval=200 \
    --distance=0 \
    --loops=100000
```

Otra alternativa consiste en especificar una duración:

```bash
sudo cyclictest \
    --mlockall \
    --smp \
    --priority=80 \
    --interval=200 \
    --distance=0 \
    --duration=10m
```

Este último comando ejecuta la prueba durante **diez minutos**.

### 3.5. Generación del histograma de latencias

Además de los valores estadísticos, resulta interesante observar la distribución de las latencias.

Un histograma permite identificar:

- La región de latencias donde se concentra la mayor cantidad de activaciones.
- La dispersión de los tiempos de respuesta.
- La presencia de eventos poco frecuentes con latencias elevadas.
- La existencia de colas largas en la distribución.

Para facilitar esta representación se utilizará el script proporcionado por Ubuntu, basado en el procedimiento publicado por el laboratorio [OSADL](https://www.osadl.org/).

Descargue el siguiente script:

```bash
wget https://ubuntu.com/real-time/docs/_downloads/3af8bfb9ac8b109e9ffb3eaaa0487e9b/histogram-plot.sh
```

Asigne permisos de ejecución:

```bash
chmod +x histogram-plot.sh
```

Ejecute:

```bash
sudo ./histogram-plot.sh
```

El script ejecuta `cyclictest` con parámetros específicos, incluyendo un histograma de 400 posiciones (`-h400`) y una duración de un minuto (`-D1m`). Posteriormente procesa los resultados mediante GNUPlot.

El resultado gráfico se almacena en:

```text
plot.png
```

Visualizarlo mediante:

```bash
xdg-open plot.png
```

Si se utiliza Ubuntu Server sin entorno gráfico, se puede copiar el archivo al computador de trabajo mediante SSH/SCP y visualizarlo allí.

**Análisis conceptual:**

En este primer experimento se debe observar cómo se distribuyen las latencias del sistema genérico. El histograma permite complementar los valores mínimo, promedio y máximo obtenidos mediante `cyclictest`.

No debe asumirse que todas las activaciones presentan una latencia similar: los procesos del sistema operativo, las interrupciones, las operaciones de entrada/salida y otras actividades pueden generar desviaciones temporales.

---

# PARTE II. Configuración de Real-Time Ubuntu

## 4. Habilitación del kernel de tiempo real en Raspberry Pi 4

Se utilizará la versión optimizada de **Real-Time Ubuntu** para Raspberry Pi.

La documentación de Canonical se encuentra disponible en:

https://ubuntu.com/real-time#:~:text=Run%20Real%2Dtime%20Ubuntu%20on%20Raspberry%20Pi

**Importante:** para Ubuntu 24.04 LTS en Raspberry Pi 4 debe utilizarse la variante `raspi` del kernel de tiempo real. No se debe instalar el kernel Real-Time genérico destinado a otras plataformas.

### 4.1. Actualizar Ubuntu

Ejecutar:

```bash
sudo apt update
sudo apt upgrade
```

Actualizar el cliente Ubuntu Pro:

```bash
sudo apt install ubuntu-pro-client
```

### 4.2. Asociar la Raspberry Pi con Ubuntu Pro

Ejecutar:

```bash
sudo pro attach
```

El sistema solicitará completar el procedimiento de asociación.

Se mostrará una dirección web y un código de identificación.

1. Abrir la dirección indicada en el terminal.
2. Iniciar sesión con la cuenta Ubuntu One.
3. Introducir el código presentado por la Raspberry Pi.
4. Seleccionar la suscripción personal gratuita.
5. Confirmar la asociación.

Una vez finalizado el procedimiento, se podrá consultar el estado de la suscripción:

```bash
sudo pro status
```

### 4.3. Habilitación del kernel Real-Time para Raspberry Pi

Ejecute:

```bash
sudo pro enable realtime-kernel --variant=raspi
```

Este comando habilita el servicio correspondiente e instala el kernel Real-Time optimizado para Raspberry Pi 4.

Al finalizar, se solicitará reiniciar el sistema.

Ejecutar:

```bash
sudo reboot
```

**Nota:** la documentación actual de Ubuntu también contempla la instalación del paquete `linux-raspi-realtime` directamente desde los repositorios en determinados escenarios. Para este laboratorio se recomienda seguir el procedimiento de Ubuntu Pro descrito anteriormente, que permite habilitar explícitamente la variante Raspberry Pi.

## 5. Verificación del kernel Real-Time

Una vez reiniciada la **Raspberry Pi**, inicie sesión y ejecute:

```bash
uname -r
```

También:

```bash
uname -a
```

La salida debe permitir identificar que el kernel cargado corresponde a la variante Real-Time de Raspberry Pi.

Por ejemplo, se debe buscar la identificación `raspi-realtime` en la versión del kernel.

También es posible consultar los paquetes instalados:

```bash
dpkg -l | grep raspi-realtime
```

**Interpretación:** esta verificación permite confirmar que el sistema ha iniciado utilizando el kernel de tiempo real y no simplemente que los paquetes correspondientes se encuentran instalados.

La diferencia es importante: instalar un kernel no significa necesariamente que ese kernel esté siendo utilizado durante la ejecución.

---

# PARTE III. Comparación experimental entre los dos kernels

## 6. **Segunda medición**: cyclictest con Real-Time Ubuntu

Una vez habilitado y verificado el kernel **Real-Time**, repita el experimento de la **sección 3**.

Ejecute:

```bash
sudo cyclictest \
    --mlockall \
    --smp \
    --priority=80 \
    --interval=200 \
    --distance=0
```

Realice también una ejecución de dos minutos:

```bash
sudo cyclictest \
    --mlockall \
    --smp \
    --priority=80 \
    --interval=200 \
    --distance=0 \
    --duration=2m
```

Observar los valores de latencia mínima, promedio y máxima.

### 8.1. Generar nuevamente el histograma

Utilizar el mismo archivo `histogram-plot.sh` empleado anteriormente:

```bash
sudo ./histogram-plot.sh
```

Se generará nuevamente:

```text
plot.png
```

Es importante mantener los mismos parámetros del experimento original para que la comparación tenga sentido.

### 8.2. Comparación conceptual

Se deben contrastar los resultados de ambos experimentos:

| Característica | Kernel genérico | Kernel Real-Time |
|---|---|---|
| Arquitectura | Raspberry Pi 4 | Raspberry Pi 4 |
| Intervalo | 200 μs | 200 μs |
| Prioridad | 80 | 80 |
| Herramienta | cyclictest | cyclictest |
| Duración del histograma | 1 minuto | 1 minuto |
| Estadística principal | Latencia máxima | Latencia máxima |
| Distribución | Histograma | Histograma |

**Aspectos que deben observarse:**

1. ¿Se reduce la latencia máxima con el kernel Real-Time?
2. ¿La distribución de latencias presenta una menor dispersión?
3. ¿Se reduce la frecuencia de eventos con latencias elevadas?
4. ¿Se observa una diferencia entre la latencia promedio y la máxima?
5. ¿Los resultados son similares cuando se repite el experimento?

Es importante tener presente que PREEMPT_RT mejora la capacidad de respuesta y la previsibilidad temporal del kernel, pero no elimina todas las fuentes de variabilidad.

Los resultados también dependen de la carga del sistema, las interrupciones, los controladores, la frecuencia del procesador, la temperatura y las características del hardware.

Por tanto, una reducción de latencia observada no constituye por sí sola una demostración formal de que una aplicación satisface todas sus restricciones temporales.

---

# PARTE IV. Desarrollo de aplicaciones de tiempo real con C/POSIX

## 9. Introducción al tutorial *First Real-Time Ubuntu Application*

La documentación de **Canonical** presenta una secuencia de ejemplos para construir aplicaciones de tiempo real:

https://ubuntu.com/real-time/docs/tutorial/first-rt-app/

El propósito de esta parte consiste en explorar cómo las características del kernel **Real-Time** pueden aprovecharse desde aplicaciones desarrolladas en C.

Los ejemplos permiten estudiar diferentes políticas de planificación, afinidad de procesador, sincronización y empaquetamiento de aplicaciones.

Antes de comenzar, instalar las herramientas de desarrollo:

```bash
sudo apt install build-essential
```

Crear un directorio de trabajo:

```bash
mkdir -p ~/rt-tutorial
cd ~/rt-tutorial
```

Los archivos fuente originales (`cfs.c`, `fifo.c`, `edf.c`, `thread-affinity.c` y `lock.c`) se pueden descargar desde los enlaces incluidos en las respectivas secciones de la documentación oficial de Canonical.

Para compilar los ejemplos que utilizan POSIX Threads se recomienda utilizar:

```bash
gcc nombre_archivo.c -o nombre_programa -pthread
```
