# Argentum Online - Grupo 5

Repositorio del TP Final de la materia Taller de Programación (TA045), cátedra Veiga, 1C2026.

## Integrantes

- Paula Consoli (108576)
- Chiara De Laurentiis (110367)
- Renata Bruno (106860)

## Instalación

El proyecto incluye un script instalador diseñado para ejecutarse en un entorno limpio (Ubuntu/Xubuntu 24.04). Este script instala todas las dependencias necesarias, compila el código en modo Release, ejecuta los tests unitarios e instala los binarios y recursos en el sistema.

Para instalar el juego, ejecutar en la raíz del repositorio:

```bash
sudo bash installer.sh
```

Una vez finalizada la instalación:
- Los ejecutables quedan en `/usr/bin`
- Los archivos de configuración en `/etc/argentum`
- Los recursos (assets) en `/var/argentum`
- Se crearán accesos directos (`server.sh` y `client.sh`) en el Escritorio.

### Ejecución

Ejecuta los scripts generados en el escritorio:
1. Abre una terminal y ejecuta: `~/Desktop/server.sh` (para levantar el servidor).
2. Abre otra terminal y ejecuta: `~/Desktop/client.sh` (para conectar el cliente).

*Nota: para cambiar el puerto o el host, se puede editar directamente esos scripts.*

## Comandos

Para compilar localmente en modo Debug, podes utilizar el `Makefile` incluido:

```bash
make compile-debug
```

Para correr los tests unitarios:

```bash
make run-tests
```

### Comandos de cierre:

- **Cliente:** para salir del juego y desconectarte, simplemente cerrar la ventana gráfica.
- **Servidor:** para matar el servidor de forma segura, escribe la letra `q` (y luego presiona Enter) en la terminal donde se está ejecutando.

## Licencias

La implementación de las clases `Queue`, `Thread`, `Socket`, `Resolver`, `LibError` y `ResolverError` está basada en el código provisto por la cátedra: https://github.com/eldipa/hands-on-threads y https://github.com/eldipa/sockets-en-cpp 

Licencia: GPL v2  