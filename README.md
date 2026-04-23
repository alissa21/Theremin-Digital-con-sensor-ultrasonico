# Theremin-Digital-con-sensor-ultrasonico
Este proyecto consiste en un sistema digital que actúa como un instrumento musical, el cual es capaz de reproducir distintas notas musicales al detectar diferentes distancias mediante un sensor HC-SR04. Utiliza un dfplayer y una tarjeta SD para guardar los archivos de audio y reproducirlos en un parlante.

# Descripción General
El sistema se compone de tres bloques principales: entradas, procesamiento y salidas.
Como entradas, utiliza un sensor ultrasónico HC-SR04 para medir distancia y dos potenciómetros que permiten ajustar el volumen y la duración de las notas.
Toda esta información es procesada por el microcontrolador(esp32-c3), que interpreta en el código los valores recibidos y determina qué nota debe reproducirse.
En el bloque de salida, el microcontrolador envía comandos al módulo DFPlayer Mini, encargado de reproducir los archivos de audio almacenados en la tarjeta SD. La señal de audio pasa luego a un amplificador, que aumenta su potencia para que pueda ser emitida con claridad a través del parlante.

# Codigo explicacion
Los archivos están ordenados en una tarjeta SD, con una carpeta para cada nota(do, do#, re, re#, etc) que son nombradas en orden : 01, 02, 03, etc, y dentro de cada carpeta 3 samples con distintas duraciones nombradas en orden 001, 002, 003, porque de esta forma los reconoce mas rapido y facil el df player. En el código el nombre de cada nota se asocia con la carpeta donde están los archivos de samples.
La escala menor pentatónica y la mayor pentatónica, siguen un patrón de que notas forman parte entre las 12 notas musicales que existen. Cuando queramos elegir la nota base, hay que buscar por el nombre y encontrar el número de la carpeta del archivo.

Con la función "Escala "podemos conseguir la escala pentatónica menor o mayor de cualquier nota, primero busca la carpeta de esa nota, y luego de acuerdo a si elegimos la escala menor o mayor, sigue el patrón y selecciona las demás carpetas las notas que formarán parte de la escala, y los guarda todos en una lista.

Con la función "altura_nota" toma la distancia que haya medido el sensor, y según en qué rango este, va a devolver una nota entre los 5 archivos que forman la Escala pentatónica de la nota que se haya elegido. 

En setup elegimos que Escala pentatónica mayor o menor de que nota queremos.
En el loop el sensor toma mediciones cada cierto tiempo y guarda la distancia, una vez sabiendo que nota se debe reproducir, se mide en qué rango está el potenciómetro de la duración, y de esta forma se elige cual sample reproducir de la carpeta. El volumen se controla midiendo cada 300ms, y si hubo un cambio real (se usa un intervalo de cambio porque el potenciómetro siempre está variando un poco su valor aunque no se lo mueva) se usa la regla de 3 simple para convertir el valor del potenciómetro en volumen.

