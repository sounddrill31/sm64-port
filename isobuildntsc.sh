#This will build isos, WIP.
mkdir sm64iso
#cp build/*_ps2/*.elf sm64iso


# if ntsc, copy ntsc system.cnf

#if pal, copy pal system.cnf

echo -n "What is the rom region $Region ? "

case $Region in

  NTSC)
    echo -n "Executing for NTSC"
    #cp isoneeds/ntsc/*.cnf sm64iso
    ;;

  Romania | Moldova)
    echo -n "Romanian"
    ;;

  Italy | "San Marino" | Switzerland | "Vatican City")
    echo -n "Italian"
    ;;

  *)
    echo -n "unknown"
    ;;
esac
