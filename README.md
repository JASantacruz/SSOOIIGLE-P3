# INSTRUCCIONES DE USO ---- SSOOIIGLE-P3

** COMPILAR **
Para compilar esta practica, bastaria con la utilizacion del comando

        ```make```

dentro del directorio raiz de la practica, esto comenzara
ejecucion de dos comandos;

    -```dirs```

creacion de los directorios obj/ y exec/,
donde iran los archivos objeto y ejecutables, respectivamente

    - ```compile```

comenzara la compilación del programa, dandola opcion de usar
el depurador GBD gracias al flag "-g" que acompaña al comando g++

Una vez hecho esto, todos estara listo para la ejecucion del programa.

** EJECUTAR **
Para ejecutar esta practica, bastaria con la utilizacion del comando

        ```make solution```

dentro del directorio raiz de la practica, esto comenzara
la ejecucionl comando

    -./exec/buscador

que comenzara la ejecucion normal de la practica.

** LIMPIAR **

En caso de querer eliminar los ficheros creados mediante la compilacion
de la practica, bastaria con ejecutar el comando

        ```make clean```

que eliminaria los directorios obj/, exec/ y todo lo que hay dentro de ellos.
