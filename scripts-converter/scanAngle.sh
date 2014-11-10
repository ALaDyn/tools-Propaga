#! /bin/bash
# Copyright 2010, 2011, 2012, 2013, 2014 Stefano Sinigardi
source ~/.bashrc
CONVERTER=/osmino/stefano/converter/converter
DATAPROC=/osmino/stefano/dataproc/dataproc
BINARYCONVERTER=/osmino/stefano/leggi_particelle/leggi_particelle
#
inputDistr=Prpout06.zpzpos
RESAMPLING_FACTOR=2
PARTICLE_TYPE=1
#1=protoni
NEW_BINARY_ENCODING=1
#######################
#mode-switch
DO_FIRST_SLICE=0
DO_SECOND_SLICE=0
DO_THIRD_SLICE=0
DO_FOURTH_SLICE=0
DO_FIFTH_SLICE=0
DO_GNUPLOT_SCRIPT=1
########################
XCOL=1
YCOL=2
ZCOL=3
PXCOL=4
PYCOL=5
PZCOL=6
PLANE_ANGLE_COL=4
SOLID_ANGLE_COL=5
ENERGY_COL=6
#######################
NBIN=100
THMIN=0
THMAX=500
MODE_CALCULATE_ENERGY_ANGLE=41
MODE_SELECT_IN_ENERGY_ONELIMIT=38
MODE_SELECT_IN_ENERGY_TWOLIMITS=5
MODE_SELECT_IN_ANGLE=20
MODE_BIN_1D=40
########################
BELOWE=5
EMEAN2=10.0
DELTAE2=2.5
EMEAN3=20.0
DELTAE3=2.5
EMEAN4=30.0
DELTAE4=2.5
ABOVEE=40.0
########################
#First slice
if (( DO_FIRST_SLICE ))
then
$CONVERTER 1 29979245800.0 ${MODE_SELECT_IN_ENERGY_ONELIMIT} ${inputDistr}.ppg -${BELOWE} ${PARTICLE_TYPE}
mv conv.${inputDistr}.ppg ${inputDistr}.below${BELOWE}MeV.ppg
$CONVERTER 1 29979245800.0 ${MODE_CALCULATE_ENERGY_ANGLE} ${inputDistr}.below${BELOWE}MeV.ppg ${PXCOL} ${PYCOL} ${PZCOL} -${PARTICLE_TYPE}
mv conv.${inputDistr}.below${BELOWE}MeV.ppg ${inputDistr}.below${BELOWE}MeV.EN-AN.ppg
$CONVERTER 1 29979245800.0 ${MODE_BIN_1D} ${inputDistr}.below${BELOWE}MeV.EN-AN.ppg ${SOLID_ANGLE_COL} ${NBIN} ${THMIN} ${THMAX}
mv conv.${inputDistr}.below${BELOWE}MeV.EN-AN.ppg ${inputDistr}.below${BELOWE}MeV.binAN.ppg 
fi
########################
#Second slice #nb: funziona solo per PARTICLE_TYPE=1, perche' la selezione in energia e' valida con questa funzione solo per protoni
if (( DO_SECOND_SLICE ))
then
$CONVERTER 1 29979245800.0 ${MODE_SELECT_IN_ENERGY_TWOLIMITS} ${inputDistr}.ppg ${EMEAN2} ${DELTAE2}
mv conv.${inputDistr}.ppg ${inputDistr}.AvE${EMEAN2}MeV-dE${DELTAE2}MeV.ppg
$CONVERTER 1 29979245800.0 ${MODE_CALCULATE_ENERGY_ANGLE} ${inputDistr}.AvE${EMEAN2}MeV-dE${DELTAE2}MeV.ppg ${PXCOL} ${PYCOL} ${PZCOL} -${PARTICLE_TYPE}
mv conv.${inputDistr}.AvE${EMEAN2}MeV-dE${DELTAE2}MeV.ppg ${inputDistr}.${EMEAN2}-${DELTAE2}.EN-AN.ppg
$CONVERTER 1 29979245800.0 ${MODE_BIN_1D} ${inputDistr}.${EMEAN2}-${DELTAE2}.EN-AN.ppg ${SOLID_ANGLE_COL} ${NBIN} ${THMIN} ${THMAX}
mv conv.${inputDistr}.${EMEAN2}-${DELTAE2}.EN-AN.ppg ${inputDistr}.${EMEAN2}-${DELTAE2}.binAN.ppg 
fi
########################
#Third slice #nb: funziona solo per PARTICLE_TYPE=1, perche' la selezione in energia e' valida con questa funzione solo per protoni
if (( DO_THIRD_SLICE ))
then
$CONVERTER 1 29979245800.0 ${MODE_SELECT_IN_ENERGY_TWOLIMITS} ${inputDistr}.ppg ${EMEAN3} ${DELTAE3}
mv conv.${inputDistr}.ppg ${inputDistr}.AvE${EMEAN3}MeV-dE${DELTAE3}MeV.ppg
$CONVERTER 1 29979245800.0 ${MODE_CALCULATE_ENERGY_ANGLE} ${inputDistr}.AvE${EMEAN3}MeV-dE${DELTAE3}MeV.ppg ${PXCOL} ${PYCOL} ${PZCOL} -${PARTICLE_TYPE}
mv conv.${inputDistr}.AvE${EMEAN3}MeV-dE${DELTAE3}MeV.ppg ${inputDistr}.${EMEAN3}-${DELTAE3}.EN-AN.ppg
$CONVERTER 1 29979245800.0 ${MODE_BIN_1D} ${inputDistr}.${EMEAN3}-${DELTAE3}.EN-AN.ppg ${SOLID_ANGLE_COL} ${NBIN} ${THMIN} ${THMAX}
mv conv.${inputDistr}.${EMEAN3}-${DELTAE3}.EN-AN.ppg ${inputDistr}.${EMEAN3}-${DELTAE3}.binAN.ppg 
fi
########################
#Fourth slice #nb: funziona solo per PARTICLE_TYPE=1, perche' la selezione in energia e' valida con questa funzione solo per protoni
if (( DO_FOURTH_SLICE ))
then
$CONVERTER 1 29979245800.0 ${MODE_SELECT_IN_ENERGY_TWOLIMITS} ${inputDistr}.ppg ${EMEAN4} ${DELTAE4}
mv conv.${inputDistr}.ppg ${inputDistr}.AvE${EMEAN4}MeV-dE${DELTAE4}MeV.ppg
$CONVERTER 1 29979245800.0 ${MODE_CALCULATE_ENERGY_ANGLE} ${inputDistr}.AvE${EMEAN4}MeV-dE${DELTAE4}MeV.ppg ${PXCOL} ${PYCOL} ${PZCOL} -${PARTICLE_TYPE}
mv conv.${inputDistr}.AvE${EMEAN4}MeV-dE${DELTAE4}MeV.ppg ${inputDistr}.${EMEAN4}-${DELTAE4}.EN-AN.ppg
$CONVERTER 1 29979245800.0 ${MODE_BIN_1D} ${inputDistr}.${EMEAN4}-${DELTAE4}.EN-AN.ppg ${SOLID_ANGLE_COL} ${NBIN} ${THMIN} ${THMAX}
mv conv.${inputDistr}.${EMEAN4}-${DELTAE4}.EN-AN.ppg ${inputDistr}.${EMEAN4}-${DELTAE4}.binAN.ppg 
fi
########################
#Fifth slice
if (( DO_FIFTH_SLICE ))
then
$CONVERTER 1 29979245800.0 ${MODE_SELECT_IN_ENERGY_ONELIMIT} ${inputDistr}.ppg ${ABOVEE} ${PARTICLE_TYPE}
mv conv.${inputDistr}.ppg ${inputDistr}.above${ABOVEE}MeV.ppg
$CONVERTER 1 29979245800.0 ${MODE_CALCULATE_ENERGY_ANGLE} ${inputDistr}.above${ABOVEE}MeV.ppg ${PXCOL} ${PYCOL} ${PZCOL} -${PARTICLE_TYPE}
mv conv.${inputDistr}.above${ABOVEE}MeV.ppg ${inputDistr}.above${ABOVEE}MeV.EN-AN.ppg
$CONVERTER 1 29979245800.0 ${MODE_BIN_1D} ${inputDistr}.above${ABOVEE}MeV.EN-AN.ppg ${SOLID_ANGLE_COL} ${NBIN} ${THMIN} ${THMAX}
mv conv.${inputDistr}.above${ABOVEE}MeV.EN-AN.ppg ${inputDistr}.above${ABOVEE}MeV.binAN.ppg 
fi
########################
########################
if (( NEW_BINARY_ENCODING ))
then
WEIGHT1_full=(`tail -1 ${inputDistr}.parameters`)
WEIGHT2_full=(`tail -6 ${inputDistr}.parameters`)
WEIGHT1=${WEIGHT1_full[0]#weight=}
WEIGHT2=${WEIGHT2_full[0]#np_over_nm=}
else
WEIGHT1=1
WEIGHT2_full=(`tail -5 ${inputDistr}.parameters`)
WEIGHT2=${WEIGHT2_full[0]#np_over_nm=}
exit
fi
########################
if (( DO_GNUPLOT_SCRIPT ))
then
 GNUPLOT_FILE_AN="spettriAngolari.plt"
 rm -f ${GNUPLOT_FILE_AN}
 touch ${GNUPLOT_FILE_AN}
 chmod 775 ${GNUPLOT_FILE_AN}
 printf "#!/gnuplot\n" >> ${GNUPLOT_FILE_AN}
 printf "FILE_IN_1='%s.below%sMeV.binAN.ppg'\n" "${inputDistr}" "${BELOWE}" >> ${GNUPLOT_FILE_AN}
 printf "FILE_IN_2='%s.%s-%s.binAN.ppg'\n" "${inputDistr}" "${EMEAN2}" "${DELTAE2}" >> ${GNUPLOT_FILE_AN}
 printf "FILE_IN_3='%s.%s-%s.binAN.ppg'\n" "${inputDistr}" "${EMEAN3}" "${DELTAE3}" >> ${GNUPLOT_FILE_AN}
 printf "FILE_IN_4='%s.%s-%s.binAN.ppg'\n" "${inputDistr}" "${EMEAN4}" "${DELTAE4}" >> ${GNUPLOT_FILE_AN}
 printf "FILE_IN_5='%s.above%sMeV.binAN.ppg'\n" "${inputDistr}" "${ABOVEE}" >> ${GNUPLOT_FILE_AN}
 printf "FILE_OUT='%s.ANGULARspectra.pdf'\n" "${inputDistr}" >> ${GNUPLOT_FILE_AN}
 printf "set terminal pdf enhanced\n" >> ${GNUPLOT_FILE_AN}
 printf "set output FILE_OUT\n" >> ${GNUPLOT_FILE_AN}
 printf "set xlabel 'Solid Angle {/Symbol W} (msr)'\n" >> ${GNUPLOT_FILE_AN}
 printf "set ylabel 'dN/d{/Symbol W} (msr^{-1})'\n" >> ${GNUPLOT_FILE_AN}
 printf "set format y '10^{%%L}'\n" >> ${GNUPLOT_FILE_AN}
 printf "set xrange[%s:%s]\n" "${THMIN}" "${THMAX}" >> ${GNUPLOT_FILE_AN}
 printf "set logscale y\n" >> ${GNUPLOT_FILE_AN}
 printf "plot FILE_IN_1 u ((\$1+\$2)*0.5):(\$3*%s*%s*%s) w histeps lt 1 lc rgb 'blue' lw 3 t 'E < %s MeV',\\" "${WEIGHT1}" "${WEIGHT2}" "${RESAMPLING_FACTOR}" "${BELOWE}" >> ${GNUPLOT_FILE_AN}
 printf "\n" >> ${GNUPLOT_FILE_AN}
 printf "FILE_IN_2 u ((\$1+\$2)*0.5):(\$3*%s*%s*%s) w histeps lt 1 lc rgb 'red' lw 3 t 'E = %s +- %s MeV',\\" "${WEIGHT1}" "${WEIGHT2}" "${RESAMPLING_FACTOR}" "${EMEAN2}" "${DELTAE2}" >> ${GNUPLOT_FILE_AN}
 printf "\n" >> ${GNUPLOT_FILE_AN}
 printf "FILE_IN_3 u ((\$1+\$2)*0.5):(\$3*%s*%s*%s) w histeps lt 1 lc rgb 'brown' lw 3 t 'E = %s +- %s MeV',\\" "${WEIGHT1}" "${WEIGHT2}" "${RESAMPLING_FACTOR}" "${EMEAN3}" "${DELTAE3}" >> ${GNUPLOT_FILE_AN}
 printf "\n" >> ${GNUPLOT_FILE_AN}
 printf "FILE_IN_4 u ((\$1+\$2)*0.5):(\$3*%s*%s*%s) w histeps lt 1 lc rgb 'green' lw 3 t 'E = %s +- %s MeV',\\" "${WEIGHT1}" "${WEIGHT2}" "${RESAMPLING_FACTOR}" "${EMEAN4}" "${DELTAE4}" >> ${GNUPLOT_FILE_AN}
 printf "\n" >> ${GNUPLOT_FILE_AN}
 printf "FILE_IN_5 u ((\$1+\$2)*0.5):(\$3*%s*%s*%s) w histeps lt 1 lc rgb 'dark-green' lw 3 t 'E > %s MeV' \n" "${WEIGHT1}" "${WEIGHT2}" "${RESAMPLING_FACTOR}" "${ABOVEE}" >> ${GNUPLOT_FILE_AN}
fi
