# Documentación: Servidor DHCP

## Descripción general
El servidor DHCP se encarga de asignar direcciones IP y otros parámetros de configuración de red a los clientes que lo solicitan, utilizando el protocolo DHCP (Dynamic Host Configuration Protocol). El servidor escucha las solicitudes de los clientes y responde con ofertas de direcciones IP disponibles, gestionando todo el ciclo de vida de una concesión de IP.

## Archivos del Servidor DHCP
El servidor DHCP está compuesto por los siguientes tres archivos:

- **servidor_dhcp.h**: Archivo de cabecera que define estructuras, constantes y prototipos de funciones.
- **servidor_dhcp.c**: Archivo de implementación que contiene la lógica de las funciones del servidor DHCP.
- **main.c**: Archivo principal que controla el flujo del servidor, manejando las solicitudes de los clientes.

## servidor_dhcp.h
Este archivo contiene las definiciones clave para el servidor DHCP.

### Estructura: `struct mensajeDhcp`
Esta estructura define un mensaje DHCP, que se utiliza tanto para las solicitudes de los clientes como para las respuestas del servidor.

**Campos principales:**
- `op`: Tipo de operación (1 = Solicitud, 2 = Respuesta).
- `htype`: Tipo de hardware (Ethernet).
- `hlen`: Longitud de la dirección de hardware.
- `xid`: ID de transacción, utilizado para correlacionar las solicitudes y respuestas entre el cliente y el servidor.
- `opciones`: Opciones DHCP (como tipo de mensaje, tiempo de arrendamiento, etc.).

### Constantes Definidas:

#### Puertos:
- **PUERTO_SERVIDOR**: Puerto donde el servidor escucha las solicitudes de los clientes (por defecto, 67).
- **PUERTO_CLIENTE**: Puerto donde el cliente recibe respuestas del servidor (por defecto, 68).

#### Tamaños y Límites:
- **BUFFER_LEN**: Tamaño del buffer para recibir los mensajes DHCP.
- **MAX_DIRECCIONES**: Máximo de direcciones IP que puede manejar el servidor.
- **TIEMPO_ARRENDAMIENTO**: Tiempo por defecto para el arrendamiento de una IP (en segundos).
- **OPCION_TIPO**: Longitud de las opciones que identifican el tipo de mensaje DHCP.

### Funciones Prototipadas:

- `int crearSocketServidor()`: Crea un socket UDP para escuchar las solicitudes DHCP de los clientes.
- `void inicializarListaIps()`: Inicializa la lista de direcciones IP disponibles para asignación.
- `int procesarSolicitud(int socketServidor, struct sockaddr_in *clienteAddr, char *buffer)`: Procesa las solicitudes DHCP Discover y Request.
- `void enviarDhcpOffer(int socketServidor, struct sockaddr_in *clienteAddr)`: Envía un mensaje DHCP Offer al cliente.
- `void enviarDhcpAck(int socketServidor, struct sockaddr_in *clienteAddr)`: Envía un mensaje DHCP Acknowledge para confirmar la asignación de IP.
- `int buscarDireccionDisponible()`: Busca una dirección IP disponible en la lista de direcciones IP del servidor.
- `int obtenerTipoMensaje(unsigned char *opciones)`: Extrae y determina el tipo de mensaje DHCP (Discover, Offer, Request, Ack, etc.).
- `void registrarConcesion(int ipAsignada)`: Registra una concesión de IP en la tabla de concesiones del servidor.

## servidor_dhcp.c
Este archivo contiene la implementación de las funciones definidas en el archivo de cabecera.

### Funciones Implementadas:

#### `int crearSocketServidor()`:
- Crea un socket UDP que permite al servidor DHCP escuchar las solicitudes de los clientes.
- Configura la opción para reutilizar la dirección (SO_REUSEADDR) y enlaza el socket al puerto estándar de DHCP (67).
- Si falla, muestra un error y termina el programa.

#### `void inicializarListaIps()`:
- Inicializa una lista de direcciones IP disponibles que el servidor puede asignar.
- En este caso, las IPs disponibles son 192.168.1.100 a 192.168.1.200, y se marcan como no asignadas.

#### `int procesarSolicitud(int socketServidor, struct sockaddr_in *clienteAddr, char *buffer)`:
- Esta función recibe y procesa las solicitudes DHCP de los clientes.
- Identifica el tipo de mensaje recibido (Discover, Request) utilizando la función obtenerTipoMensaje.
- Si el mensaje es un DHCP Discover, el servidor prepara y envía una oferta de IP (DHCP Offer).
- Si el mensaje es un DHCP Request, el servidor confirma la asignación de IP (DHCP Acknowledge).

#### `void enviarDhcpOffer(int socketServidor, struct sockaddr_in *clienteAddr)`:
- Envía un mensaje DHCP Offer al cliente que envió un mensaje DHCP Discover.
- Selecciona una dirección IP disponible y la ofrece al cliente, junto con otros parámetros (como la máscara de subred).
- El mensaje incluye la dirección MAC del cliente y las opciones de arrendamiento.

#### `void enviarDhcpAck(int socketServidor, struct sockaddr_in *clienteAddr)`:
- Envía un mensaje DHCP Acknowledge para confirmar la asignación de la dirección IP solicitada por el cliente.
- El cliente puede comenzar a usar la dirección IP una vez que recibe este mensaje.

#### `int buscarDireccionDisponible()`:
- Busca en la lista de direcciones IP del servidor una dirección que esté disponible para asignación.
- Devuelve la dirección IP si se encuentra una disponible, o -1 si no hay más IPs disponibles.

#### `int obtenerTipoMensaje(unsigned char *opciones)`:
- Extrae y determina el tipo de mensaje DHCP a partir del campo de opciones del mensaje recibido.
- Los tipos posibles son DHCP Discover (1), DHCP Offer (2), DHCP Request (3), y DHCP Ack (5).

#### `void registrarConcesion(int ipAsignada)`:
- Registra la asignación de una dirección IP a un cliente.
- Marca la IP como "concesionada" y registra la hora de arrendamiento para llevar el control del tiempo de la concesión.

## main.c
Este archivo contiene el punto de entrada principal para el servidor DHCP. Gestiona la recepción de solicitudes de los clientes y las respuestas correspondientes.

### Proceso General:

#### Inicialización:
- Se crea el socket del servidor para escuchar en el puerto DHCP (67).
- Se inicializa la lista de direcciones IP disponibles para asignación.

#### Bucle principal:
- El servidor entra en un bucle infinito donde espera solicitudes de los clientes.
- Utiliza `procesarSolicitud` para recibir y manejar los mensajes DHCP Discover y Request.
- Dependiendo del tipo de solicitud, envía una oferta o un reconocimiento.

#### Manejo de IPs:
- El servidor busca una dirección IP disponible para cada solicitud y registra la asignación una vez confirmada.

### Flujo del programa:
1. Inicializa el servidor y la lista de direcciones IP.
2. Escucha continuamente en el puerto 67 para las solicitudes DHCP de los clientes.
3. Procesa cada solicitud y envía las respuestas correspondientes:
   - DHCP Offer: Oferta de una IP.
   - DHCP Ack: Confirmación de la asignación.
4. Maneja las direcciones IP y las concesiones de manera eficiente.
"""