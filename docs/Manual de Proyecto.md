# Manual de Proyecto — Argentum Online (Grupo 5)

*TP Final de la materia Taller de Programación (TA045), cátedra Veiga, 1C2026.*

## Integrantes

- Paula Consoli (108576)
- Chiara De Laurentiis (110367)
- Renata Bruno (106860)

## División del trabajo

Nuestro grupo se compuso de 3 personas en lugar de 4. Para la organización de tareas, usamos la división propuesta por nuestra correctora y realizamos un sorteo para definir qué integrante realizaría cada parte.

La división final fue la siguiente:

- **Chiara** se encargó de implementar la parte SDL del cliente
- **Renata** se encargó de implementar la parte de QT del cliente, la comunicación entre cliente y servidor, threads, protocolo y testing
- **Paula** se encargó de implementar la parte de lógica del servidor

## Organización semanal y plan inicial

Seguimos en líneas generales el cronograma propuesto por nuestra correctora (setup y diseño inicial, conexión cliente-servidor, features incrementales semana a semana), aunque arrancamos con aproximadamente una semana de atraso respecto al plan original. Esto corrió en cascada el resto del cronograma y de "entregas" intermedias, aunque para la entrega final logramos terminar con todas las features especificadas en el enunciado implementadas (aunque más ajustadas con el tiempo de lo que nos hubiera gustado)


## Herramientas utilizadas

Como IDE, las 3 integrantes utilizamos Visual Studio Code con las extensiones de C++ (clangd y CMake Tools).

En cuanto a linters y herramientas para mantener coherencia en el estilo de código, configuramos `clang-format` con un archivo `.clang-format` en la raíz del repositorio. Durante el desarrollo, los warnings del compilador se trataron como errores (`-Werror`) para evitar acumular problemas. También usamos AddressSanitizer (ASAN) y UndefinedBehaviorSanitizer (UBSAN), que se pueden activar desde CMake, para detectar errores de memoria durante el desarrollo.

Alineándonos con el desarrollo de los trabajos individuales, también configuramos pre-commit hooks basados en los propuestos por la cátedra (adaptándolos para nuestro proyecto), que se ejecutan automáticamente antes de cada commit y corren tres checks: 

- `clang-format` v14.0.0 para verificar el formato del código (mencionado anteriormente)
- `cpplint` v1.6.0 para detectar problemas de estilo según las guías de Google
- `cppcheck` v2.13.0 para análisis estático

Esto nos permitió mantener el código consistente sin tener que revisarlo manualmente.

El sistema de build es CMake 3.24+, con un `Makefile` como wrapper para los comandos más frecuentes (`compile-debug`, `run-tests`). Los tests unitarios se escribieron con GoogleTest, integrado al proyecto vía `FetchContent`. 

El control de versiones se manejó con Git en GitHub, usando ramas por feature (`feat/...`), fixes (`fix/...`) y refactors (`refactor/...`), con una rama `develop` de integración en la que cada una mergeaba sus propias ramas antes de mergear a `main` (rama estable de la entrega).


## Documentación usada

Para **SDL2** ... 

Para **Qt5** consultamos la documentación oficial en https://doc.qt.io/archives/qt-5.15/

Se consultó **CPPReference** https://cppreference.com/ para dudas sobre C++, en especial para el uso de smart pointers (`unique_ptr` y `shared_ptr`)

El parser usado para los archivos `.toml` fue **toml++**, uno de los más comúnmente utilizados. Se consultó documentación en https://marzer.github.io/tomlplusplus/ 

Para **GoogleTest** se consultó https://google.github.io/googletest/ 


## Puntos más problemáticos

- **Integración cliente-servidor**: cliente y servidor se desarrollaron en paralelo con distintas suposiciones sobre el protocolo y en algunos caso el formato de mensajes. Cada vez que el servidor agregaba un campo nuevo, el cliente debía actualizarse, y sincronizar eso fue un problema en varios momentos (puntualmente, en situaciones donde el cliente gráfico necesitaba, para mostrar cosas por pantalla, campos que el servidor aún no enviaba)

- El mayor desafío fue el **atraso inicial**, que achicó el margen para las últimas features (persistencia y clanes) y dejó menos tiempo de integración entre los tres módulos (servidor, SDL y Qt) antes de la entrega final, lo que nos provocó no poder chequear todo tan minuciosamente y con tranquilidad como hubieramos querido


## Errores conocidos 

La mayoría de los errores (que identificamos post-entrega) se deben a falta de tiempo y de chequeos de cambios que hicimos o integramos a último momento.

1. **Persistencia del banco no implementada**: actualmente el banco del jugador (oro y objetos depositados) no se persiste junto al resto del estado del personaje. Queda planteado como mejora para una próxima iteración.
2. **Persistencia en instalación limpia**: al instalar con el installer, el servidor intentaba escribir save.json en /usr/bin/ (su directorio de trabajo), donde el usuario no tiene permisos de escritura, fallando con "no se pudo abrir save.json.tmp". Se resolvería leyendo la ruta de guardado desde la variable de entorno ARGENTUM_SAVE_FILE, que server.sh configura apuntando a /var/argentum/save.json, y dando los permisos de escritura correspondientes.
3. **Tests de ataque rotos por actualización de status**: en desarrollo, los códigos de error al chequear fair play y diferencias de nivel entre jugadores para el ataque fueron ignorados, ya que esos valores de pusieron en el game_config.toml de manera que no sean un inconveniente para poder probar el ataque. Cuando antes de la entrega se cambiaron los valores para coincidir con el enunciado y los outcomes requeridos (no poder atacar/ser atacado si nivel <= 12 o diferencia de niveles >10), tuvimos que agregar nuevos STATUS (para el "error" del ataque se identifique correctamente en la interfaz) a último momento, olvidando cambiar esos status en dos tests. Estos fallan porque siguen esperando el código genérico INVALID_TARGET, cuando ya se distingue NEWBIE_PROTECTION y LEVEL_DIFFERENCE.

Los errores 2 y 3 mencionados se corrigieron en la branch `develop`, aunque se los identificó y corrigió post-entrega (la branch estable usada para la entrega es `main`).

## ¿Qué cambiaríamos?
A nivel *código*:

- Definir el protocolo binario y el formato del snapshot desde el principio, antes de arrancar con cliente y servidor por separado. Evitaría la mayoría de los conflictos de integración.
- Aprovechar más el polimorfismo y abstraer más el diseño. En este momento, hay partes del sistema más acopladas de lo que nos gustaría. En un futuro querríamos que sea más extensible, más fácil de continuar para una persona externa y menos dependiente de nuestro conocimiento sobre la arquitectura/formato/flujo de mensajes.

A nivel *organización*:

- Fijar sesiones de integración cliente-servidor más frecuentes, para no chocarnos tanto con problemas de formatos
- Ponerle más tiempo y centrarnos más en el diagramado inicial. Muchos de nuestros problemas se hubieran resuelto más fácil si le hubieramos dedicado más al planeamiento inicial
- Documentar los contratos del protocolo en un archivo compartido para que cliente y servidor no tuvieran suposiciones distintas

## ¿Qué debería darse en Taller?
Nos hubiera gustado un poco más de foco en temas de diseño; quizás se podría incluir una práctica para repasar patrones de diseño útiles que nos podrían ayudar en el diagramado de nuestro proyecto y en qué escenarios pueden aplicarse