============================================================
        Archivo README para el emulador de Vircon32
           (versión 25.1.19) escrito por Carra
============================================================

¿Qué es esto?

    Esto es un emulador de Vircon32, una consola virtual de 32
    bits, para PC. Te permite jugar a juegos de Vircon32 en tu
    ordenador.

    La versión actual se considera terminada, salvo cualquier
    corrección de errores que se pueda necesitar en el futuro. 
    Ya soporta todas las características de la consola, y el
    emulador ya tiene todas las funciones extra que estaban
    previstas (configuraciones, interfaz gráfica, etc).

------------------------------------------------------------

Instalación 

    Según el sistema operativo que uses el proceso para poder
    ejecutar el emulador será distinto:
    
    Windows:
    --------
    Basta con descomprimir el ZIP en cualquier carpeta, ya
    que incluye todas las librerías DLL necesarias.
      
    Linux:
    --------
    Instala el paquete DEB o RPM (según tu sistema) usando
    los comandos que correspondan:
      DEB: (1) Instalar paquete: sudo dpkg -i <paquete>
           (2) Instalar dependencias: sudo apt-get install -f
      RPM: Instalar el paquete junto con sus dependencias:
           sudo yum localinstall <paquete>
              
    MacOS:
    --------
    Primero descomprime el ZIP a cualquier carpeta con permisos
    de escritura. Para instalar las dependencias se usa el
    gestor de paquetes Homebrew (se puede instalar desde su web).
    Los comandos para instalarlas son:
      brew install sdl2
      brew install sdl2_image
      brew install openal-soft
      brew install freealut
      brew install tinyxml2
    
------------------------------------------------------------

Ventana del programa 

    Normalmente la interfaz gráfica está escondida, pero se
    muestra cuando el ratón está sobre la ventana del emulador.
    También se puede mostrar manualmente con la tecla escape.

    Usa el ratón para navegar por los menús y usar las opciones.

------------------------------------------------------------

Cómo cargar juegos

    Vircon32 es un emulador, así que para que funcione necesitas
    insertar un cartucho virtual, también llamado archivo ROM, y
    luego encender la consola.

    En esta consola la ranura del cartucho se bloquea cuando la
    consola está encendida, con lo cual para quitar un cartucho
    o cambiarlo por otro tendrás que apagar primero la consola.

    También puedes abrir un juego simplemente con doble click en
    el archivo ROM *.v32 y, en el diálogo "Abrir con...", elegir
    el emulador de Vircon32 como programa para abrirlo.
    
------------------------------------------------------------

Controles

    Por defecto sólo el mando 1 está conectado, y está mapeado
    al teclado de la siguiente forma:

      - Cruceta: Teclas de dirección (flechas)
      - Botones L,R: Teclas 'Q','W'
      - Botones Y,X: Teclas 'A','S'
      - Botones B,A: Teclas 'Z','X'
      - Botón Start: Tecla 'Return'

    El emulador también soporta el uso de uno o más joysticks
    (esta consola tiene 4 puertos para mandos). Los controles de
    teclado y joysticks se pueden editar usando un segundo
    programa incluido con este emulador, llamado EditControls.
    Lee la ayuda de EditControls para aprender cómo configurar
    tus dispositivos.
  
------------------------------------------------------------

Cómo usar tarjetas de memoria

    Cuando algún cartucho de Vircon32 necesita guardar/cargar
    datos entre sesiones, necesitará una tarjeta de memoria para
    preservar la información guardada.
    
    Hay 2 modos de gestionar las tarjetas de memoria. Puedes
    elegir cuál usar desde el menú de opciones.
    
    Automática (opción por defecto):
    
      El emulador crea automáticamente una tarjeta de memoria
      nueva para cada juego que abramos. Las guarda en la
      subcarpeta "Cards" del propio emulador.
      
      Este modo es el mismo que usa el núcleo Vircon32 en
      RetroArch. También funcionan así muchos otros emuladores.
    
    Manual:
    
      El emulador no creará ni usará tarjetas automáticamente.
      En vez de eso, desd el menú de tarjeta de memoria podrás
      crear nuevas tarjetas y decidir cuándo ponerlas y quitarlas.
      
      Este modo refleja la manera en que usaríamos una consola
      física, manejando las tarjetas y cartuchos nosotros mismos.
      
    vez que un juego escriba en ella, esa tarjeta sólo la
    reconocerá ese mismo juego, así que necesitarás una tarjeta
    para cada juego en donde quieras guardar partida.

    Si usas el modo manual, ¡ten cuidado de usar la tarjeta de
    memoria correcta en cada cartucho! Cada juego debería
    comprobar si los datos de la tarjeta pertenecen a otros
    cartuchos, pero no hay garantías de que realmente lo hagan.

------------------------------------------------------------

Cómo usar savestates
    
    Los savestates (estados) te permiten guardar el estado de
    la consola durante el juego en cualquier momento, y luego
    cargarlo más tarde. Así puedes continuar jugando después,
    incluso en juegos que no permiten guardar la partida.
    
    Hay 4 ranuras para estados en cada juego. Por defecto se
    usa la ranura 1 pero puedes elegir otra usando los menús.
    ¡Ten cuidado de usar la ranura correcta cuando cargas o
    guardas estado, pues estas acciones no se pueden deshacer!
    
    Ten en cuenta que los estados no guardan el contenido de
    la pantalla, que en Vircon32 es persistente. Esto es así a
    propósito para reducir tamaño y complejidad de los estados.
    Casi todos los juegos redibujan la pantalla cada frame,
    pero en unos pocos casos puede que tras cargar un estado
    la pantalla no se actualice inmediatamente. Especialmente
    si guardaste un estado en menús o transiciones de pantalla.
    
------------------------------------------------------------

Novedades en la versión 25.1.19
    
  - En Windows se arreglan los problemas que impedían acceder
    a archivos si su ruta tenía caracteres no ingleses.
  - Se corrige la velocidad de emulación en pantallas que usen
    una tasa de refresco por encima de los 60 Hz.

------------------------------------------------------------

Licencia
    
    Este programa es gratuito y de código abierto. Se ofrece bajo
    la licencia BSD de 3 cláusulas, cuyo texto completo es el
    siguiente:
    
    Copyright 2021-2025 Carra.
    Todos los derechos reservados.
    
    La redistribución y el uso en las formas de código fuente y
    binario, con o sin modificaciones, están permitidos siempre
    que se cumplan las siguientes condiciones:

    1. Las redistribuciones del código fuente deben conservar el
    aviso de copyright anterior, esta lista de condiciones y el
    siguiente descargo de responsabilidad.
    
    2. Las redistribuciones en formato binario deben reproducir
    el aviso de copyright anterior, esta lista de condiciones y
    el siguiente descargo de responsabilidad en la documentación
    y/u otros materiales suministrados con la distribución.
    
    3. Ni el nombre de los titulares de derechos de autor ni los
    nombres de sus colaboradores pueden usarse para apoyar o
    promocionar productos derivados de este software sin permiso
    específico previo y por escrito.

    ESTE SOFTWARE SE SUMINISTRA POR LOS PROPIETARIOS DEL
    COPYRIGHT Y COLABORADORES “COMO ESTÁ” Y CUALQUIER GARANTÍAS
    EXPRESA O IMPLÍCITA, INCLUYENDO, PERO NO LIMITADO A, LAS
    GARANTÍAS IMPLÍCITAS DE COMERCIALIZACIÓN Y APTITUD PARA UN
    PROPÓSITO PARTICULAR SON RECHAZADAS. EN NINGÚN CASO LOS
    PROPIETARIOS DEL COPYRIGHT Y COLABORADORES SERÁN RESPONSABLES
    POR NINGÚN DAÑO DIRECTO, INDIRECTO, INCIDENTAL, ESPECIAL,
    EJEMPLAR O COSECUENCIAL (INCLUYENDO, PERO NO LIMITADO A, LA
    ADQUISICIÓN O SUSTITUCIÓN DE BIENES O SERVICIOS; LA PÉRDIDA
    DE USO, DE DATOS O DE BENEFICIOS; O INTERRUPCIÓN DE LA
    ACTIVIDAD EMPRESARIAL) O POR CUALQUIER TEORÍA DE
    RESPONSABILIDAD, YA SEA POR CONTRATO, RESPONSABILIDAD ESTRICTA
    O AGRAVIO (INCLUYENDO NEGLIGENCIA O CUALQUIER OTRA CAUSA) QUE
    SURJA DE CUALQUIER MANERA DEL USO DE ESTE SOFTWARE, INCLUSO SI
    SE HA ADVERTIDO DE LA POSIBILIDAD DE TALES DAÑOS.
