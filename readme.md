# Servidor DHCP en C

## Descripción
Este proyecto implementa un servidor **DHCP** simple en lenguaje C que asigna direcciones IP dinámicamente a los clientes que lo solicitan. Utiliza la API de sockets para crear un servidor en la red local que responde a los mensajes **DHCPDISCOVER** y **DHCPREQUEST** de los clientes, enviando **DHCPOFFER** y **DHCPACK** en respuesta.

## Requisitos

- Sistema operativo Linux.
- Biblioteca estándar de sockets (arpa/inet.h, sys/socket.h).
- Permisos de superusuario para enlazar al puerto 67 (puerto privilegiado).

## Características
- Asignación dinámica de direcciones IP a clientes **DHCP**.
- Soporte para las fases **DISCOVER**, **OFFER**, **REQUEST**, **ACK**.
- El servidor escucha en la red local en el puerto 67 y responde a las solicitudes de los clientes en el puerto 68.
- Diseño básico y extensible.

## Uso (instalación)

### Clona el repositorio

```bash
git clone https://github.com/JuanMaLopez2/Proyecto_dhcp.git
```
### Compilar el servidor

Compila el código utilizando `gcc` y ejecuta el servidor desde la terminal:

```bash
#ingresa a la carpeta del servidor

cd servidor

#compila el servidor
gcc -o servidor_dhcp main.c servidor_dhcp.c

#ejecuta el servidor

sudo ./servidor_dhcp
```

### Compilar el cliente

Compila el código utilizando `gcc` y ejecuta el codigo:

```bash
#ingresa a la carpeta del cliente

cd cliente

#compila el servidor
gcc -o cliente_dhcp main.c cliente_dhcp.c

#ejecuta el servidor

sudo ./cliente_dhcp
```



## Autores
- **Juan Manuel López Sánchez**
- ****

## Referencias
- ISC DHCP Overview (https://github.com/isc-projects/dhcp)
- RFC 2131: Dynamic Host Configuration Protocol (DHCP) (https://datatracker.ietf.org/doc/html/rfc2131)

## Importante
Los archivos **DocCliente_dhcp.md** y **DocServidor_dhcp.md** contienen la documentación detallada de cada una de las carpetas, tanto para el servidor como para el cliente sobre su estructura.
