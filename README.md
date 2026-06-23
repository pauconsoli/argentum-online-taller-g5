# Argentum Online - Grupo 5

Repositorio del TP Final de la materia Taller de Programación (TA045), cátedra Veiga, 1C2026.

## Integrantes

- Paula Consoli (108576)
- Chiara De Laurentiis (110367)
- Renata Bruno (106860)

## Instalación

El proyecto incluye un script instalador diseñado para ejecutarse en un entorno limpio (Ubuntu/Xubuntu 24.04). Este script instala todas las dependencias necesarias, compila el código en modo Release, ejecuta los tests unitarios e instala los binarios y recursos en el sistema.

Para instalar el juego, ejecutar en la raíz del repositorio o donde este el archivo `installer.sh`:

```bash
sudo bash installer.sh
```

Una vez finalizada la instalación:
- Los ejecutables quedan en `/usr/bin`
- Los archivos de configuración en `/etc/argentum`
- Los recursos (assets) en `/var/argentum`
- Se crearán accesos directos (`server.sh` y `client.sh`) en el escritorio.

### Ejecución

Ejecutá los scripts generados en el escritorio. En esa carpeta:
1. Abrí una terminal y ejecutá: `./server.sh` (para levantar el servidor).
2. Abrí otra terminal y ejecutá: `./client.sh` (para conectar el cliente).

*Nota: para cambiar el puerto, el host o levantar el server con Valgrind se puede editar directamente esos scripts.*

## Comandos

Para compilar localmente en modo Debug, podes utilizar el `Makefile` incluido:

```bash
make compile-debug
```

Para correr los tests unitarios:

```bash
make run-tests
```

O directamente el binario, con filtros:

```bash
# Todos los tests (game, world, protocolo)
./build/argentum_tests

# Solo los tests del protocolo binario
./build/argentum_tests --gtest_filter='ProtocolFixture.*'

# Un test específico
./build/argentum_tests --gtest_filter='ProtocolFixture.LoginRoundTrip'

# Listar todos los tests disponibles sin correrlos
./build/argentum_tests --gtest_list_tests
```

Los tests del protocolo cubren round-trip de los mensajes principales
(login, chat, attack con tipo de arma, snapshot, match_joined con restore,
inventory con uint64 grandes, etc.) usando `socketpair()` para simular un
par cliente-servidor sin abrir red real.
Para levantar Server y Client (en la raíz):

```bash
./build/argentum_server 8080

./build/argentum_client_qt
```
Para levantar Server con Valgrind:

```bash
valgrind --leak-check=full --track-origins=yes --show-leak-kinds=all ./build/argentum_server 8080
```

### Comandos de cierre:

- **Cliente:** para salir del juego y desconectarte, simplemente cerrar la ventana gráfica.
- **Servidor:** para matar el servidor de forma segura, escribir la letra `q` (y luego presiona Enter) en la terminal donde se está ejecutando.

## Licencias

La implementación de las clases `Queue`, `Thread`, `Socket`, `Resolver`, `LibError` y `ResolverError` está basada en el código provisto por la cátedra: https://github.com/eldipa/hands-on-threads y https://github.com/eldipa/sockets-en-cpp 

Licencia: GPL v2  

---

## Comandos del juego

### Movimiento y acciones básicas

| Acción | Control |
|--------|---------|
| Mover personaje | Teclas de dirección (↑ ↓ ← →) |
| Atacar | Click izquierdo sobre otro jugador |
| Abrir chat | `Enter` → escribir mensaje → `Enter` para enviar |

### Comandos de inventario

| Comando / Acción | Descripción |
|------------------|-------------|
| `/tomar` | Recoge el item del suelo donde estás parado |
| `/tirar` | Tira al piso el item seleccionado (ver abajo cómo seleccionarlo) |
| Click izquierdo sobre item en inventario | Equipar o usar el item (las pociones se consumen inmediatamente) |
| Click derecho sobre item en inventario | Selecciona el item para `/tirar` (se marca con borde celeste) |

---

## Persistencia

El server guarda el estado de los jugadores periódicamente en un archivo
`save.json` (formato autorizado por la cátedra como alternativa al binario
del enunciado). El save se actualiza:

- Automáticamente cada **30 segundos** desde adentro del game loop.
- Al cerrar el server limpio con `q`.

La escritura es **atómica**: se escribe a un archivo temporal `.tmp` y se
hace `rename` POSIX al definitivo. Si el server crashea mid-write, el save
anterior queda intacto.

Al volver a levantar el server, se recrean automáticamente los matches que
estaban activos al cierre anterior. Cuando un jugador joinea un match con
su mismo nick y nombre de match, el server detecta el save y lo restaura
con sus stats, gold e inventario — sin pedir raza/clase de nuevo.

La identidad del save es la combinación `(nick, match_name)`. El
`match_id` es runtime y no sobrevive entre runs, por eso usamos el nombre
del match (que es estable).
