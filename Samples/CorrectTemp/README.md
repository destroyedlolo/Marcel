This sample directory contains a test I made to compensate the raw temperature measured by my daughter bedroom probe impacted by the temperature of the attic on the other side of the wall.


----------
**Usage :**

    ./Marcel -f Samples/CorrectTemp/TestCorrection.conf

 - **Reference** : reference temperature
 - **Grenier Sud** : attic temperature
 - **Chambre Oceane** : bedroom temperature.


----------
**Result :**

The result is published in a topic <topic>/cmp in CSV format as following

    Grenier- Chambre , function - Ref , Average - Ref, Chambre
    -0.5625 , 0.21154644241758 ,  0.211181640625, 16.25

As example, it can be retrieved using 

    mosquitto_sub -t 'maison/Temperature/Chambre Oceane/cmp'

> Written with [StackEdit](https://stackedit.io/).
