# Documentacion Técnica - Grupo 5 

*TP Final de la materia Taller de Programación (TA045), cátedra Veiga, 1C2026.*

## Integrantes

- Paula Consoli (108576)
- Chiara De Laurentiis (110367)
- Renata Bruno (106860)

## Objetivo

Este documento resume la arquitectura técnica de nuestro proyecto de TP final. La documentacion prioriza los componentes mas importantes del sistema, con el objetivo de ser entendida por otro desarrollador que busque conocer nuestra implementación.

## General

El proyecto esta implementado en C++20 y se organiza con tres areas principales:

- `common/`: infraestructura, contratos, archivos compartidos entre cliente y servidor
- `client/`: cliente de juego, con interfaz SDL y una capa Qt para lobby/menú
- `server/`: lógica del juego, modelo y clases/entidades del sistema, arquitectura del servidor, partidas, mundo y persistencia

La arquitectura sigue un modelo de servidor **autoritativo**:

- El cliente solo envia intenciones o acciones
- El servidor valida esas acciones y actualiza el estado real
- El servidor envia snapshots y updates a los clientes
- El cliente renderiza el estado recibido

## Módulos Principales

### `common/`

Contiene los componentes compartidos que definen el contrato entre ambos extremos:

- `Socket`: wrapper de TCP, implementación provista por la cátedra (licencia incluida en el *README.md*)
- `Thread`: abstraccion base para threads cooperativos, implementación provista por la cátedra (licencia incluida en el *README.md*)
- `Queue`: cola bloqueante multiproductor/multiconsumidor, implementación provista por la cátedra (licencia incluida en el *README.md*)
- `protocol_constants.h`: opcodes del protocolo binario
- `common/commands/*`: comandos cliente -> servidor.
- `common/updates/*`: mensajes servidor -> cliente.

Este módulo reduce duplicacion: cliente y servidor hablan exactamente el mismo protocolo porque comparten las estructuras y constantes.


### `client/`

El cliente se organiza en tres capas:

1. **Capa de red** (`Client`, `ClientProtocol`): gestiona el socket, serializa comandos para enviar y corre un thread interno que recibe updates del servidor y los encola en `received_updates`
2. **Capa Qt** (`client/qt/`): implementa el lobby (login, lista de matches, selección de raza/clase) con widgets Qt. Al unirse a una partida, hace *handoff* (pasa el `Client` ya conectado a la capa SDL)
3. **Capa SDL** (`client/sdl/`): corre el loop visual del juego. Sus componentes principales son:
  - `GameClient`: orquesta el loop principal, procesa input y renderiza
  - `GameState`: estado visual del jugador y el mundo (posición, stats, inventario, entidades)
  - `ServerUpdateHandler`: consume la cola de updates del servidor y los aplica sobre `GameState` y `ClientMap`
  - `WorldRenderer` + renderers individuales: toman el `GameState` y lo dibujan en pantalla
  - `InputHandler`: traduce eventos SDL a `InputAction` abstractas
  - `HUD`, `MiniChat`, `ClanPanel`: elementos de UI para el cliente

La UI no decide el estado real del juego: muestra lo que el servidor confirma. El flujo es siempre: 

**input → Client::do_*() → servidor → GameUpdate → ServerUpdateHandler → GameState → render**

### `server/`

Es el núcleo del juego. Sus componentes más importantes son:

- Gestión de conexiones y sesiones de clientes
- Lógica del juego, modelo del mundo y entidades
- Sistema de partidas y ciclo de vida del gameloop
- Persistencia y restauración del estado

## Arquitectura del Servidor

### Responsabilidades de `Server`

`Server` es el punto de coordinación principal. Sus responsabilidades son:

- Registrar clientes conectados
- Validar logins y evitar nicks duplicados
- Crear y listar matches
- Permitir que un jugador ingrese o salga de un match
- Delegar comandos al `Match` correspondiente
- Enviar updates a un jugador o hacer broadcast
- Persistir y restaurar el estado

Métodos clave:

- `login()`
- `create_match()`
- `join_match()`
- `leave_match()`
- `push_command_to_match()`
- `send_world_map_to()`
- `snapshot()`
- `save_state()`
- `find_player_save()`

### Responsabilidades de `BasicMatch`

Cada `BasicMatch` encapsula una partida activa. Contiene:

- Identidad del match
- Capacidad máxima
- Lista de jugadores conectados a esa partida
- Una `Queue<std::unique_ptr<ClientCommand>>` para recibir comandos
- Un **World** propio

El método mas importante es `tick()`:

- Vacía la cola de comandos
- Ejecuta cada `ClientCommand` sobre el `World`
- Obtiene una lista de `GameUpdate`
- Distribuye cada update por medio de un **broadcast** (a un jugador específico o a todos los conectados, dependiendo de la naturaleza del update)

Esta separación es importante porque evita ejecutar lógica de juego directamente dentro del thread de red.

## Comunicación Entre Threads y Concurrencia

### Flujo principal de entrada

1. El cliente envía un **opcode** y su **payload** por socket
2. `ReceiverThread` lee el socket y valida el estado de la sesión
3. Si el comando corresponde al lobby, interactúa directamente con `Server`
4. Si el jugador está dentro de un match, el thread arma un `ClientCommand`
5. El comando se encola en `BasicMatch::command_queue`.
6. `GameLoopThread` ejecuta ese comando en el tick siguiente

### Flujo principal de salida

1. La ejecución del comando produce `GameUpdate`
2. El update se dirige a un jugador o se broadcastea
3. El `Match` o el `Server` lo encolan en `PlayerConnection::send_queue`
4. `SenderThread` toma el update, lo serializa con `ServerProtocol` y lo manda por socket
5. El cliente lo deserializa y actualiza su estado local en base a lo que recibió

### Diagrama de secuencia: comunicación

![alt text](<diagrams/Secuencia - Comunicación de threads.jpg>)

### Estado de sesión de un jugador

Cada **PlayerConnection** mantiene:

- `player_id`
- `nick`
- `current_match_id`
- `state`
- `send_queue`

Los estados posibles son:

- `CONNECTED`: conectado pero no autenticado
- `AUTHENTICATED`: logueado en lobby
- `IN_MATCH`: dentro de una partida
- `DISCONNECTING`: en cierre

Esto permite que `ReceiverThread` acepte solo ciertos opcodes segun el contexto actual.

## Protocolo de Comunicación

### Tipo de transporte

La comunicacion usa sockets TCP. No hay mensajes independientes a nivel de transporte: el protocolo define una secuencia de bytes sobre un *stream*.

### Estructura general de un mensaje

Cada mensaje se codifica como:

- `opcode` de 1 byte
- Payload, cuyo formato depende del opcode

El framing no usa un encabezado global con longitud total. En cambio, cada lado sabe como leer el payload porque:

- Primero lee el opcode
- Luego interpreta el resto segun el tipo del mensaje.

### Tipos primitivos

Los primitivos se codifican de la siguiente forma:

- `u8`: 1 byte.
- `u16`: 2 bytes en big endian.
- `u32`: 4 bytes en big endian.
- `u64`: 8 bytes como dos `u32` en big endian.
- `i32`: se serializa como 32 bits y se transmite en big endian.
- `string`: `u16 length` + `length` bytes.

### OpCodes cliente -> servidor

Principales opcodes:

- `0x01 LOGIN`
- `0x02 LIST_MATCHES`
- `0x03 CREATE_MATCH`
- `0x04 JOIN_MATCH`
- `0x05 SELECT_RACE_CLASS`
- `0x10 MOVE`
- `0x11 ATTACK`
- `0x12 MEDITATE`
- `0x13 PICK_UP`
- `0x14 DROP_ITEM`
- `0x15 EQUIP_ITEM`
- `0x16 INTERACT`
- `0x17 CLAN`
- `0x18 RESURRECT`
- `0x19 CHEAT`
- `0x20 CHAT`
- `0x21 PRIVATE_CHAT`
- `0xFE LEAVE_MATCH`
- `0xFF DISCONNECT`

### OpCodes servidor -> cliente

Principales opcodes:

- `0x81 LOGIN_OK`
- `0x82 MATCH_LIST`
- `0x83 MATCH_CREATED`
- `0x84 MATCH_JOINED`
- `0x85 SNAPSHOT`
- `0x86 PLAYER_JOINED`
- `0x87 PLAYER_LEFT`
- `0x88 PLAYER_SPAWNED`
- `0x89 WORLD_MAP`
- `0x90 MOVED`
- `0x91 STATS`
- `0x92 DEATH`
- `0x93 REVIVE`
- `0x94 ATTACKED`
- `0x95 MEDITATE`
- `0xA0 INVENTORY`
- `0xA1 CATALOG`
- `0xA2 NPC_INTERACT`
- `0xB0 CHAT_MSG`
- `0xB1 SYSTEM_MSG`
- `0xC0 CLAN_RESULT`
- `0xC1 CLAN_REVIEW`
- `0xEE ERROR`

### Ejemplos de mensajes

#### *Login*

Cliente -> servidor

- `opcode = 0x01`
- `nick: string`

Servidor -> cliente

- `opcode = 0x81`
- `player_id: u32`

#### *Join match*

Cliente -> servidor

- `opcode = 0x04`
- `match_id: u32`

Servidor -> cliente

- `opcode = 0x84`
- `match_id: u32`
- `your_player_id: u32`
- `was_restored: u8`
- `restored_race: u8`
- `restored_klass: u8`

#### *Chat privado*

Cliente -> servidor

- `opcode = 0x21`
- `target_nick: string`
- `text: string`

## Formato de Archivos

### Archivos de configuración

El proyecto usa TOML (**.toml**) para los archivos que definen valores del juego.

Archivos principales:

- `common/config/game_config.toml`
- `common/config/items.toml`
- `common/config/npcs.toml`
- `common/config/map.toml`
- `client/config/sdl_config.toml`

#### `game_config.toml`

Modela parametros globales del juego:

- Multiplicadores por raza o bonus por clase
- Spawn inicial,
- Reglas de oro, experiencia y combate
- Parámetros de NPCs
- Velocidad del game loop
- Demás constantes y restricciones

Se organiza en tablas como:

- `[race.human]`
- `[class.mage]`
- `[world]`
- `[inventory]`
- `[gold]`
- `[experience]`
- `[combat]`
- `[clan]`
- `[npc]`
- `[server]`

#### `items.toml`

Define el catálogo de items. Usa tablas `items.<id>`:

```toml
[items.sword]
name = "Espada"
type = "weapon"
min_damage = 2
max_damage = 5
ranged = false
price = 10
```

Según el tipo del item aparecen campos adicionales:

- Armas: `min_damage`, `max_damage`, `ranged`
- Defensivos: `slot`, `min_defense`, `max_defense`
- Armas mágicas: `spell_name`, `heal/damage`, `mana_cost`
- Consumibles: `type`, `restore`

#### `npcs.toml`

Usa una tabla raiz por NPC:

```toml
[goblin]
name = "Goblin"
level = 4
max_hp = 100
defense = 1
agility = 3
min_damage = 15
max_damage = 20
attack_range = 5
zones = ["all"]
```

El campo `zones` permite restringir spawns por tipo de zona, por ejemplo `all` o `dungeon`.

#### `map.toml`

Es el archivo mas importante de configuracion estructural del mundo. Contiene:

- El tamaño del mapa
- La grilla completa de tiles (`rows`)
- Las zonas especiales (`[[zones]]`)
- Los objetos iniciales en el piso del mapa (`[[ground_items]]`).

Ejemplo simplificado:

```toml
[map]
width = 100
height = 70
rows = [
  "#####",
  "#...#",
  "#####"
]

[[zones]]
type = "city"
name = "Ullathorpe"
x = 1
y = 1
width = 22
height = 17

[[ground_items]]
type = "gold"
amount = 25
x = 5
y = 19
```

Las `rows` codifican el terreno por caracteres. Por ejemplo:

- `#`: piedra bloqueante,
- `.`: pasto,
- `T`: árbol bloqueante,
- `~`: agua bloqueante,
- `-`, `:`, `C`, `A`: elementos de ciudad,
- `d`, `D`, `E`, `P`, `*`, `<`: elementos de dungeon.

### Persistencia: `save.json`

La persistencia runtime usa JSON. El server:

- Intenta cargarlo al arrancar, *si existe estado persistido*
- Guarda automaticamente cada 30 segundos
- Vuelve a guardarlo al cierre limpio
- Escribe primero a `save.json.tmp` y luego hace `rename()` atomico, para evitar guardados corruptos

#### *Diagrama de secuencia: guardado*

![alt text](<diagrams/Secuencia - Guardado y persistencia.jpg>)

#### *Estructura del archivo*

Raíz:

- `version`
- `saved_at_unix`
- `players`
- `matches`

Ejemplo:

```json
{
  "version": 1,
  "saved_at_unix": 1760000000,
  "players": [
    {
      "nick": "Renata",
      "match_name": "Partida 1",
      "race": 0,
      "class": 1,
      "level": 5,
      "xp": 1800,
      "hp": 120,
      "max_hp": 150,
      "mp": 60,
      "max_mp": 90,
      "gold": 250,
      "pos": {
        "x": 12,
        "y": 34
      },
      "inventory": [
        {
          "item_name": "Espada",
          "quantity": 1,
          "equipped": true
        },
        {
          "item_name": "Pocion de vida",
          "quantity": 2,
          "equipped": false
        }
      ]
    }
  ],
  "matches": [
    {
      "name": "Partida 1",
      "max_players": 4
    }
  ]
}
```

#### *Identidad persistente*

El save de un jugador se identifica por la dupla:

- `nick`
- `match_name`

No se usa `match_id` porque ese valor es runtime y puede cambiar entre ejecuciones.

#### *Restauración*

Cuando un jugador entra a un match:

- El servidor busca una entrada en cache por `(nick, match_name)`
- Si existe, marca el join como restaurado
- Encola un `RestorePlayerCommand`
- El comando recrea *raza, clase, stats, oro, posición e inventario*

#### *Diagrama de secuencia: join y restauración*


![alt text](<diagrams/Secuencia - REstauración.jpg>)

< *Nota 1:* > los items persistidos se reconstruyen por nombre usando `ItemRegistry`. Si un item ya no existe en el registry, se saltea.

< *Nota 2:* > en el estado actual del proyecto, la persistencia del `Banco` de la partida no se encuentra implementada. Es un feature a implementar en el futuro.

## Métodos y Clases más importantes

### Clases del lado servidor

- `Server`: orquestacion general.
- `BasicMatch`: partida activa y cola de comandos.
- `World`: estado del mundo y reglas principales.
- `PlayerConnection`: estado de sesion y cola de salida por jugador.
- `ReceiverThread`: frontera entre socket y comandos del dominio.
- `SenderThread`: frontera entre updates del dominio y socket.
- `GameLoopThread`: reloj del servidor y generacion de snapshots.
- `Persister`: carga/guardado y cache del save.
- `WorldSerializer`: conversion entre `WorldSnapshot` y JSON

### Métodos destacados

- `Server::run()`: arranque, threads principales y apagado
- `Server::snapshot()`: extrae un modelo persistible del estado del juego
- `Server::save_state()`: controla el autosave y los errores de IO
- `ReceiverThread::run()`: dispatcher de opcodes segun estado de sesión
- `ReceiverThread::handle_join_match()`: join, envío de mapa y posible restauración
- `BasicMatch::tick()`: ejecuta comandos y distribuye updates
- `GameLoopThread::run()`: actualiza el mundo, emite snapshots y dispara autosave
- `Persister::try_load()`: carga inicial
- `Persister::save()`: escritura atómica del save
- `RestorePlayerCommand::execute()`: restaura jugadores persistidos

### Diagrama de la arquitectura del servidor

![alt text](<diagrams/Diagrama arquiectura del server-2.jpg>)

### Diagrama del modelo y sus relaciones principales

![alt text](<diagrams/Modelo - Relaciones principales.jpg>)

## Decisiones de Diseño Relevantes

### Cola entre red y lógica

La decisión más importante es no ejecutar lógica pesada en los hilos de red. Los sockets:

- Leen y traducen
- Encolan comandos
- Envían respuestas ya preparadas

La simulacion vive en el gameloop. Esto simplifica consistencia y reduce acoplamiento.

### Mundo por match

Cada `Match` posee su propio `World`. Esto aísla partidas entre si:

- Jugadores de distintos matches no comparten entidades ni estado
- El gameloop puede iterar partida por partida
- La persistencia puede reconstruir matches completos

### Persistencia por snapshot

El sistema no persiste objetos internos complejos tal como viven en memoria. En cambio:

- Extrae un `WorldSnapshot`
- Serializa datos de alto nivel
- Reconstruye el estado desde ese snapshot

Esto desacopla la persistencia de la representación interna exacta del dominio.

