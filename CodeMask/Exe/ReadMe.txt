Calling with parameters
Encode : Exe name -e Directory Full path without the Forward Slash at the end

Decode : Exe name -e File Name with Full path




Encoding ( The Folder Should have all the separate Mask images named as Mask1.png Mask2.png .... (Mask1.png is the Main Mask) )

>CodeMask.exe  -e "C:\Mask"
>CodeMask.exe  -e ".\Mask"


Decoding (All decoded masks will be stored in the same path as the coded Mask.png)

> CodeMask.exe -d "C:\Mask\Mask.png"
> CodeMask.exe -d ".\Mask\Mask.png"
