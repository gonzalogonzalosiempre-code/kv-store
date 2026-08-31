[README.md](https://github.com/user-attachments/files/31625370/README.md)
# KV Store en C++ — Almacén clave-valor persistente, concurrente y en red

Implementación desde cero, en C++20, de un sistema de almacenamiento clave-valor inspirado en Redis: estructura de datos propia (sin `std::unordered_map`), persistencia en disco mediante write-ahead log, servidor TCP con sockets crudos de Winsock, y soporte para múltiples clientes simultáneos mediante hilos y sincronización con mutex.

Este proyecto se construyó como ejercicio de aprendizaje profundo de sistemas, gestión de memoria, redes y concurrencia en C++ — priorizando entender cada pieza a bajo nivel antes que usar librerías que abstraigan el problema.

---

## Demo en video

- 🎥 **[Explicación completa (30 min)]()** — recorrido detallado de arquitectura, decisiones de diseño y funcionamiento interno.
- 🎥 **[Demo rápida (9 min)]()** — explicación resumida y prueba en vivo del sistema funcionando.

---

## Características

- **Hash table con chaining**, implementada desde cero (sin contenedores de la STL para el almacenamiento principal)
- **Persistencia real** mediante write-ahead log — los datos sobreviven a un cierre o crash del proceso
- **Servidor TCP** sobre Winsock, con protocolo de texto plano propio (`SET` / `GET` / `DEL`)
- **Concurrencia real**: múltiples clientes atendidos simultáneamente, cada uno en su propio hilo, compartiendo el mismo almacén de datos de forma segura
- **Validación de entrada** contra comandos malformados (protección explícita contra un bug de desbordamiento encontrado y corregido durante el desarrollo — ver sección de bugs)

---

## Arquitectura

```
Cliente (PowerShell / cualquier cliente TCP)
        │
        │  texto plano: "SET perro labrador"
        ▼
┌───────────────────────┐
│   Servidor TCP         │  Winsock: socket → bind → listen → accept
│   (Server.h)           │  Un hilo (std::jthread) por cliente conectado
└───────────┬────────────┘
            │  ParserCommand() → Eject()
            ▼
┌───────────────────────┐
│   HashTable             │  Estructura de datos + persistencia
│   (Hash.h)              │  Protegida con std::mutex
├───────────────────────┤
│  Buckets: vector<Nodo*> │  Chaining para colisiones
│  Write-ahead log (.log) │  Reconstrucción de estado al iniciar
└───────────────────────┘
```

---

## Cómo funciona

### La hash table

Cada clave se convierte en un índice mediante una **función hash polinomial**:

```cpp
hash = hash * 31 + caracter_ascii
index = hash % tamaño_tabla
```

Se usa el primo **31** como multiplicador porque distribuye mejor las claves entre los buckets que una simple suma de caracteres — sin la multiplicación por un primo, palabras con las mismas letras en distinto orden (como "ROMA" y "AMOR") colisionarían siempre en el mismo índice.

Las colisiones se resuelven con **chaining**: cada posición del vector de buckets es la cabeza de una lista enlazada. Se eligió chaining sobre direccionamiento abierto (open addressing) porque es más simple de razonar correctamente — evita los problemas de clustering y de manejo especial de borrado que trae el open addressing, a cambio de una localidad de memoria ligeramente peor.

### Persistencia — write-ahead log

Cada operación que modifica el estado (`SET`, `DEL`) se escribe **inmediatamente** a un archivo en disco, en formato:

```
SET|clave|valor
DEL|clave
```

Al arrancar, el programa relee ese archivo completo, línea por línea, y reconstruye el estado en memoria reproduciendo cada operación en el orden en que ocurrió — igual que hace un motor de base de datos real. Este orden es crítico: si una clave fue actualizada y luego eliminada, el estado final debe reflejar exactamente esa secuencia, no una versión resumida.

La separación entre `SET`/`DEL` (públicos, que modifican memoria **y** escriben al log) y sus versiones privadas de solo-memoria fue una decisión de diseño necesaria: sin ella, la reconstrucción al arrancar volvería a escribir cada línea que ya existía en el archivo, haciéndolo crecer sin control en cada reinicio.

**Trade-off aceptado:** el log crece indefinidamente y cada arranque relee el historial completo — para un proyecto de este alcance es aceptable; en un sistema de producción real esto se resolvería con *snapshotting* periódico (guardar el estado completo y descartar el log anterior), una extensión identificada pero no implementada en esta versión.

### El servidor TCP

Construido directamente sobre la API de Winsock (sin librerías de abstracción de red), siguiendo la secuencia estándar de sockets:

`WSAStartup` → `socket` → `bind` → `listen` → `accept` → `recv`/`send` → `closesocket` → `WSACleanup`

El protocolo es texto plano sobre TCP: el cliente manda una línea como `"GET perro"`, el servidor la parsea, ejecuta la operación correspondiente sobre la hash table, y responde con el resultado como texto.

### Concurrencia

Cada cliente aceptado se atiende en su propio hilo (`std::jthread`, desconectado con `.detach()` para no bloquear al hilo principal, que vuelve inmediatamente a esperar nuevas conexiones). Todos los hilos comparten la **misma** instancia de `HashTable` — es el propósito central de un KV store, que los datos sean compartidos entre clientes.

Ese acceso compartido se protege con un `std::mutex` interno a `HashTable`, usado mediante `std::lock_guard` dentro de `SET`, `GET` y `DEL`. Puede pensarse como un guardaespaldas: solo un hilo a la vez puede estar "hablando" con los datos protegidos — cualquier otro que llegue mientras tanto espera su turno, evitando que dos hilos reconecten punteros o lean memoria que otro está modificando al mismo tiempo (una condición de carrera).

**Trade-off aceptado:** el mutex es global a toda la tabla, no granular por bucket — más simple de razonar y garantiza corrección, a cambio de menor paralelismo real bajo carga muy alta (dos hilos operando en buckets distintos igual deben esperarse entre sí). Una mejora futura razonable sería un lock por bucket.

---

## Un bug real, encontrado y corregido

Durante el desarrollo, se descubrió que un comando malformado (`SET perro`, sin el valor a guardar) provocaba un **crash completo del servidor** — no un error controlado, sino un `Assertion failed` por acceso fuera de rango en un `std::vector`.

**Causa raíz:** el parser buscaba un segundo espacio en el comando para delimitar el valor. Cuando ese espacio no existía, `std::string::find` devolvía `std::string::npos` (el valor máximo representable de un entero sin signo). Al hacer aritmética con ese valor (`npos + 1`), el resultado se desbordaba silenciosamente, produciendo posiciones de memoria inválidas — sin lanzar ninguna excepción hasta que, más adelante, se intentaba acceder a un elemento del vector de comandos que nunca llegó a existir.

**Corrección:** validación explícita de la posición devuelta por `find` antes de usarla, rechazando el comando con un mensaje de error controlado (`"ERR unknow command"`) en lugar de intentar procesarlo con datos incompletos.

Este hallazgo es la razón por la que el proyecto valida activamente su entrada en el parser — cualquier sistema que reciba datos de una fuente externa (un cliente de red, en este caso) no puede asumir que esos datos llegan bien formados.

---

## Tecnologías y conceptos aplicados

| Área | Detalle |
|---|---|
| Estructuras de datos | Hash table propia, chaining, listas enlazadas |
| Gestión de memoria | Punteros crudos (`new`/`delete`), prevención de leaks, dangling pointers y use-after-free |
| Persistencia | Write-ahead log, serialización con delimitadores, reconstrucción de estado |
| Redes | Winsock, sockets TCP, arquitectura cliente-servidor, diseño de protocolo propio |
| Concurrencia | `std::jthread`, `std::mutex`, `std::lock_guard`, `std::ref` |
| Robustez | Validación de entrada, manejo explícito de errores en cada capa |
| Herramientas | Git (branching, merge), GitHub, VS Code + MinGW-w64/GCC, GDB |

---

## Cómo compilar y ejecutar

Requiere MinGW-w64 (GCC) con soporte de C++20 y la librería `ws2_32` de Winsock (Windows).

```bash
g++ -std=c++20 -g -Wall -Wextra src/*.cpp -Iinclude -o kv-store.exe -lws2_32
./kv-store.exe
```

El servidor escucha por defecto en el puerto **8080**. Puede conectarse con cualquier cliente TCP que envíe texto plano terminado en salto de línea — por ejemplo, desde PowerShell:

```powershell
$socket = New-Object System.Net.Sockets.TcpClient("localhost", 8080)
$stream = $socket.GetStream()
$writer = New-Object System.IO.StreamWriter($stream)
$writer.AutoFlush = $true
$reader = New-Object System.IO.StreamReader($stream)

$writer.WriteLine("SET perro labrador")
$reader.ReadLine()
```

### Comandos soportados

| Comando | Formato | Descripción |
|---|---|---|
| `SET` | `SET clave valor` | Crea o actualiza una clave |
| `GET` | `GET clave` | Obtiene el valor de una clave |
| `DEL` | `DEL clave` | Elimina una clave |

---

## Posibles extensiones futuras

- Snapshotting periódico del estado, para evitar releer el log completo en cada arranque
- Locks por bucket en lugar de un mutex global, para mayor paralelismo
- Protocolo binario tipo RESP (el de Redis) en lugar de texto plano
- Política de expiración de claves (TTL)
- Benchmark de throughput bajo carga concurrente

---

## Notas sobre el proceso de desarrollo

Este proyecto se construyó de forma incremental por fases (estructura de datos → persistencia → red → concurrencia), con revisión de código y depuración activa en cada etapa. El historial de commits refleja ese proceso real, incluyendo la corrección de bugs de memoria (dangling pointers, use-after-free, memory leaks) y de robustez (el desbordamiento descrito arriba) a medida que se fueron descubriendo.
