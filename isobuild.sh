#This will build isos, WIP.
mkdir sm64iso
#cp build/*_ps2/*.elf sm64iso


# if ntsc, copy ntsc system.cnf

#if pal, copy pal system.cnf

echo -n "What is the rom region $Region ? Options: NTSC, PAL, JP. "

case $Region in

  NTSC)
    echo -n "Executing for NTSC"
    #cp isoneeds/ntsc/*.cnf sm64iso
    ;;

  PAL)
    echo -n "Executing for PAL"
    #cp isoneeds/pal/*.cnf sm64iso
    
    ;;

  JP")
    echo -n "Executing for JP-NTSC"
    ;;

  *)
    echo -n "unknown"
    echo -n "Invalid options entered, correct options are: NTSC, PAL, JP"
    ;;
esac
