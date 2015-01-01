#! /bin/bash
# Copyright 2010, 2011, 2012, 2013, 2014 Stefano Sinigardi
source ~/.bashrc
CONVERTER=/osmino/stefano/converter/converter
DATAPROC=/osmino/stefano/dataproc/dataproc
BINARYCONVERTER=/osmino/stefano/leggi_particelle/leggi_particelle
#
#
#
inputDistr=Elpout12
##nb: gli ElpoutXX.bin sono tutti "old_binary_encoding"
NEW_BINARY_ENCODING=1
RESAMPLING_FACTOR=2
TYPE_PARTICLE=3
#protoni (1) o elettroni (3)
########################
BINARY_CONVERSION_REQUIRED=0
BINARY_SWAP_REQUIRED=1
CLEAN_BINARY_OUT_REQUIRED=0
ASCII_OUT_REQUIRED=1
########################
PRESELECTION_ZPZpositives_REQUIRED=0
########################
#Energies in MeV
EMIN=0
EMAX=10
ECUT=0.001
NBIN_ENERGY=10000
#nb: if ECUT < 0 it cuts below |ECUT|, otherwise it cuts above
CUT_EN_REQUIRED=0
CONVERT_EN_REQUIRED=0
BINNING_EN_REQUIRED=0
PLOT_EN_REQUIRED=1
########################
#Angle in radians
#ANMIN=-0.50
#ANMAX=0.50
ANMIN=-1.57
ANMAX=1.57
NBIN_ANGLE=3140
CONVERT_AN_REQUIRED=0
BINNING_AN_REQUIRED=1
PLOT_AN_REQUIRED=1
########################
if (( BINARY_CONVERSION_REQUIRED ))
then
 if [[ ! -f ./${inputDistr}.bin ]]
 then
    printf "Missing binary distribution file \n"
    exit
 fi
else
 if [[ ! -f ./${inputDistr}.ppg || ! -f ./${inputDistr}.parameters ]]
 then
    printf "Missing parameters file or distribution file \n"
    exit
 fi
fi
#######################
if (( BINARY_CONVERSION_REQUIRED ))
then
 $BINARYCONVERTER ${inputDistr}.bin 2 ${BINARY_SWAP_REQUIRED} ${NEW_BINARY_ENCODING} ${CLEAN_BINARY_OUT_REQUIRED} ${ASCII_OUT_REQUIRED}
mv ${inputDistr}.bin.parameters ${inputDistr}.parameters
 if (( CLEAN_BINARY_OUT_REQUIRED )) ; then mv ${inputDistr}.bin_7_out ${inputDistr}.clean_bin ; fi
 if (( ASCII_OUT_REQUIRED ))
  then
  mv ${inputDistr}.bin.ascii ${inputDistr}.ascii
  MODE_FROM_ASCII_TO_PPG=6
  $CONVERTER 1 1 ${MODE_FROM_ASCII_TO_PPG} ${inputDistr}.ascii
  mv conv.${inputDistr}.ascii ${inputDistr}.ppg
  fi
fi
#######################
if (( NEW_BINARY_ENCODING ))
then
WEIGHT1_full=(`tail -1 ${inputDistr}.parameters`)
WEIGHT2_full=(`tail -6 ${inputDistr}.parameters`)
#questa versione con 'sed' funziona anche per gli elettroni che hanno peso negativo
WEIGHT1=`echo "${WEIGHT1_full[0]#weight=}" | sed 's/-//g' `
#WEIGHT1=${WEIGHT1_full[0]#weight=}
WEIGHT2=${WEIGHT2_full[0]#np_over_nm=}
else
WEIGHT1=1
WEIGHT2_full=(`tail -5 ${inputDistr}.parameters`)
WEIGHT2=${WEIGHT2_full[0]#np_over_nm=}
exit
fi
ENERGYCOL=4
ANGLEXCOL=4
ANGLEYCOL=5
MODE_BINNING_NOT_WEIGHTED_4c=17
MODE_BINNING_NOT_WEIGHTED_5c=34
MODE_CALCULATE_ENERGY=27
MODE_CUT_ENERGY=38
MODE_CALCULATE_ANGLE=32
MODE_SELECT_IN_A_CONE=20
MODE_REMOVE_NEGATIVES=35
CONE_APERTURE_MRAD=50
ZCOL=3
PZCOL=6
#######################
if (( PRESELECTION_ZPZpositives_REQUIRED ))
then
$CONVERTER 1 1 ${MODE_REMOVE_NEGATIVES} ${inputDistr}.ppg ${ZCOL}
mv conv.${inputDistr}.ppg ${inputDistr}.zpos.ppg
$CONVERTER 1 1 ${MODE_REMOVE_NEGATIVES} ${inputDistr}.zpos.ppg ${PZCOL}
mv conv.${inputDistr}.zpos.ppg ${inputDistr}.zpzpos.ppg
inputDistr=${inputDistr}.zpzpos
fi
#######################
if (( CONVERT_EN_REQUIRED ))
 then
 $CONVERTER 1 29979245800.0 ${MODE_CALCULATE_ENERGY} ${inputDistr}.ppg ${TYPE_PARTICLE}
 mv conv.${inputDistr}.ppg ${inputDistr}.ENERGY.ppg
 $CONVERTER 1 1 ${MODE_SELECT_IN_A_CONE} ${inputDistr}.ppg 0 ${CONE_APERTURE_MRAD}
 mv conv.${inputDistr}.ppg ${inputDistr}.${CONE_APERTURE_MRAD}mrad.ppg
 $CONVERTER 1 29979245800.0 ${MODE_CALCULATE_ENERGY} ${inputDistr}.${CONE_APERTURE_MRAD}mrad.ppg ${TYPE_PARTICLE}
 mv conv.${inputDistr}.${CONE_APERTURE_MRAD}mrad.ppg ${inputDistr}.${CONE_APERTURE_MRAD}mrad.ENERGY.ppg
fi
if (( BINNING_EN_REQUIRED ))
 then
 $CONVERTER 1 1 ${MODE_BINNING_NOT_WEIGHTED_4c} ${inputDistr}.ENERGY.ppg ${ENERGYCOL} ${NBIN_ENERGY} ${EMIN} ${EMAX}
 mv conv.${inputDistr}.ENERGY.ppg ${inputDistr}.ENERGY.binned-${EMIN}-${EMAX}.ppg
 $CONVERTER 1 1 ${MODE_BINNING_NOT_WEIGHTED_4c} ${inputDistr}.${CONE_APERTURE_MRAD}mrad.ENERGY.ppg ${ENERGYCOL} ${NBIN_ENERGY} ${EMIN} ${EMAX}
 mv conv.${inputDistr}.${CONE_APERTURE_MRAD}mrad.ENERGY.ppg ${inputDistr}.${CONE_APERTURE_MRAD}mrad.ENERGY.binned-${EMIN}-${EMAX}.ppg
fi
########################
if (( CONVERT_AN_REQUIRED ))
 then
 $CONVERTER 1 1 ${MODE_CALCULATE_ANGLE} ${inputDistr}.ppg 6
 mv conv.${inputDistr}.ppg ${inputDistr}.ANGLE.ppg
fi
if (( BINNING_AN_REQUIRED ))
 then
 $CONVERTER 1 1 ${MODE_BINNING_NOT_WEIGHTED_5c} ${inputDistr}.ANGLE.ppg ${ANGLEXCOL} ${NBIN_ANGLE} ${ANMIN} ${ANMAX} 1
 mv conv.${inputDistr}.ANGLE.ppg ${inputDistr}.ANGLE.binnedX-${ANMIN}-${ANMAX}.ppg
 $CONVERTER 1 1 ${MODE_BINNING_NOT_WEIGHTED_5c} ${inputDistr}.ANGLE.ppg ${ANGLEYCOL} ${NBIN_ANGLE} ${ANMIN} ${ANMAX} 1
 mv conv.${inputDistr}.ANGLE.ppg ${inputDistr}.ANGLE.binnedY-${ANMIN}-${ANMAX}.ppg
fi
#######################
if (( CUT_EN_REQUIRED ))
then
 $CONVERTER 1 29979245800.0 ${MODE_CUT_ENERGY} ${inputDistr}.ppg ${ECUT} ${TYPE_PARTICLE}
 mv conv.${inputDistr}.ppg ${inputDistr}.cut${ECUT}MeV.ppg
 inputDistr2=${inputDistr}.cut${ECUT}MeV
 if (( CONVERT_EN_REQUIRED ))
 then
  $CONVERTER 1 29979245800.0 ${MODE_CALCULATE_ENERGY} ${inputDistr2}.ppg ${TYPE_PARTICLE}
  mv conv.${inputDistr2}.ppg ${inputDistr2}.ENERGY.ppg
  $CONVERTER 1 1 ${MODE_SELECT_IN_A_CONE} ${inputDistr2}.ppg 0 ${CONE_APERTURE_MRAD}
  mv conv.${inputDistr2}.ppg ${inputDistr2}.${CONE_APERTURE_MRAD}mrad.ppg
  $CONVERTER 1 29979245800.0 ${MODE_CALCULATE_ENERGY} ${inputDistr2}.${CONE_APERTURE_MRAD}mrad.ppg ${TYPE_PARTICLE}
  mv conv.${inputDistr2}.${CONE_APERTURE_MRAD}mrad.ppg ${inputDistr2}.${CONE_APERTURE_MRAD}mrad.ENERGY.ppg
 fi
 if (( BINNING_EN_REQUIRED ))
 then
  $CONVERTER 1 1 ${MODE_BINNING_NOT_WEIGHTED_4c} ${inputDistr2}.ENERGY.ppg ${ENERGYCOL} ${NBIN_ENERGY} ${EMIN} ${EMAX}
  mv conv.${inputDistr2}.ENERGY.ppg ${inputDistr2}.ENERGY.binned-${EMIN}-${EMAX}.ppg
  $CONVERTER 1 1 ${MODE_BINNING_NOT_WEIGHTED_4c} ${inputDistr2}.${CONE_APERTURE_MRAD}mrad.ENERGY.ppg ${ENERGYCOL} ${NBIN_ENERGY} ${EMIN} ${EMAX}
  mv conv.${inputDistr2}.${CONE_APERTURE_MRAD}mrad.ENERGY.ppg ${inputDistr2}.${CONE_APERTURE_MRAD}mrad.ENERGY.binned-${EMIN}-${EMAX}.ppg
 fi
 if (( CONVERT_AN_REQUIRED ))
 then
  $CONVERTER 1 1 ${MODE_CALCULATE_ANGLE} ${inputDistr2}.ppg 6
  mv conv.${inputDistr2}.ppg ${inputDistr2}.ANGLE.ppg
 fi
 if (( BINNING_AN_REQUIRED ))
 then
  $CONVERTER 1 1 ${MODE_BINNING_NOT_WEIGHTED_5c} ${inputDistr2}.ANGLE.ppg ${ANGLEXCOL} ${NBIN_ANGLE} ${ANMIN} ${ANMAX} 1
  mv conv.${inputDistr2}.ANGLE.ppg ${inputDistr2}.ANGLE.binnedX-${ANMIN}-${ANMAX}.ppg
  $CONVERTER 1 1 ${MODE_BINNING_NOT_WEIGHTED_5c} ${inputDistr2}.ANGLE.ppg ${ANGLEYCOL} ${NBIN_ANGLE} ${ANMIN} ${ANMAX} 1
  mv conv.${inputDistr2}.ANGLE.ppg ${inputDistr2}.ANGLE.binnedY-${ANMIN}-${ANMAX}.ppg
 fi
fi
########################
if (( PLOT_EN_REQUIRED ))
then
 GNUPLOT_FILE_EN="spettroEnergetico.plt"
 rm -f ${GNUPLOT_FILE_EN}
 touch ${GNUPLOT_FILE_EN}
 chmod 775 ${GNUPLOT_FILE_EN}
 printf "#!/gnuplot\n" >> ${GNUPLOT_FILE_EN}
 printf "FILE_IN_1='%s.ENERGY.binned-%s-%s.ppg'\n" "${inputDistr}" "${EMIN}" "${EMAX}" >> ${GNUPLOT_FILE_EN}
 printf "FILE_IN_2='%s.%smrad.ENERGY.binned-%s-%s.ppg'\n" "${inputDistr}" "${CONE_APERTURE_MRAD}" "${EMIN}" "${EMAX}" >> ${GNUPLOT_FILE_EN}
 printf "FILE_OUT='%s.ENERGYspectra.pdf'\n" "${inputDistr}" >> ${GNUPLOT_FILE_EN}
 printf "set terminal pdf enhanced\n" >> ${GNUPLOT_FILE_EN}
 printf "set output FILE_OUT\n" >> ${GNUPLOT_FILE_EN}
 printf "set xlabel 'E (MeV)' \n" >> ${GNUPLOT_FILE_EN}
 printf "set ylabel 'dN/dE (MeV^{-1})'\n" >> ${GNUPLOT_FILE_EN}
 printf "set format y '10^{%%L}'\n" >> ${GNUPLOT_FILE_EN}
 printf "set xrange[%s:%s]\n" "${EMIN}" "${EMAX}" >> ${GNUPLOT_FILE_EN}
 printf "set logscale y\n" >> ${GNUPLOT_FILE_EN}
 printf "plot FILE_IN_1 u ((\$1+\$2)*0.5):(\$3*%s*%s*%s) w histeps lt 1 lc rgb 'blue' lw 3 t 'full spectrum',\\" "${WEIGHT1}" "${WEIGHT2}" "${RESAMPLING_FACTOR}" >> ${GNUPLOT_FILE_EN}
 printf "\n" >> ${GNUPLOT_FILE_EN}
 printf "FILE_IN_2 u ((\$1+\$2)*0.5):(\$3*%s*%s*%s) w histeps lt 1 lc rgb 'dark-green' lw 3 t 'inside 50mrad' \n" "${WEIGHT1}" "${WEIGHT2}" "${RESAMPLING_FACTOR}" >> ${GNUPLOT_FILE_EN}
 if (( CUT_EN_REQUIRED ))
 then
  GNUPLOT_FILE_EN="spettroEnergetico_cut${ECUT}MeV.plt"
  rm -f ${GNUPLOT_FILE_EN}
  touch ${GNUPLOT_FILE_EN}
  chmod 775 ${GNUPLOT_FILE_EN}
  printf "#!/gnuplot\n" >> ${GNUPLOT_FILE_EN}
  printf "FILE_IN_1='%s.ENERGY.binned-%s-%s.ppg'\n" "${inputDistr2}" "${EMIN}" "${EMAX}" >> ${GNUPLOT_FILE_EN}
  printf "FILE_IN_2='%s.%smrad.ENERGY.binned-%s-%s.ppg'\n" "${inputDistr2}" "${CONE_APERTURE_MRAD}" "${EMIN}" "${EMAX}" >> ${GNUPLOT_FILE_EN}
  printf "FILE_OUT='%s.ENERGYspectra.pdf'\n" "${inputDistr2}" >> ${GNUPLOT_FILE_EN}
  printf "set terminal pdf enhanced\n" >> ${GNUPLOT_FILE_EN}
  printf "set output FILE_OUT\n" >> ${GNUPLOT_FILE_EN}
  printf "set xlabel 'E (MeV)' \n" >> ${GNUPLOT_FILE_EN}
  printf "set ylabel 'dN/dE (MeV^{-1})'\n" >> ${GNUPLOT_FILE_EN}
  printf "set format y '10^{%%L}'\n" >> ${GNUPLOT_FILE_EN}
  printf "set xrange[%s:%s]\n" "${EMIN}" "${EMAX}" >> ${GNUPLOT_FILE_EN}
  printf "set logscale y\n" >> ${GNUPLOT_FILE_EN}
  printf "plot FILE_IN_1 u ((\$1+\$2)*0.5):(\$3*%s*%s*%s) w histeps lt 1 lc rgb 'blue' lw 3 t 'full spectrum',\\" "${WEIGHT1}" "${WEIGHT2}" "${RESAMPLING_FACTOR}" >> ${GNUPLOT_FILE_EN}
  printf "\n" >> ${GNUPLOT_FILE_EN}
  printf "FILE_IN_2 u ((\$1+\$2)*0.5):(\$3*%s*%s*%s) w histeps lt 1 lc rgb 'dark-green' lw 3 t 'inside 50mrad' \n" "${WEIGHT1}" "${WEIGHT2}" "${RESAMPLING_FACTOR}" >> ${GNUPLOT_FILE_EN}
 fi
fi
########################
if (( PLOT_AN_REQUIRED ))
then
 GNUPLOT_FILE_AN="spettroAngolare.plt"
 rm -f ${GNUPLOT_FILE_AN}
 touch ${GNUPLOT_FILE_AN}
 chmod 775 ${GNUPLOT_FILE_AN}
 printf "#!/gnuplot\n" >> ${GNUPLOT_FILE_AN}
 printf "FILE_IN_1='%s.ANGLE.binnedX-%s-%s.ppg'\n" "${inputDistr}" "${ANMIN}" "${ANMAX}" >> ${GNUPLOT_FILE_AN}
 printf "FILE_IN_2='%s.ANGLE.binnedY-%s-%s.ppg'\n" "${inputDistr}" "${ANMIN}" "${ANMAX}" >> ${GNUPLOT_FILE_AN}
 printf "FILE_OUT='%s.ANGULARspectra.pdf'\n" "${inputDistr}" >> ${GNUPLOT_FILE_AN}
 printf "set terminal pdf enhanced\n" >> ${GNUPLOT_FILE_AN}
 printf "set output FILE_OUT\n" >> ${GNUPLOT_FILE_AN}
 printf "set xlabel 'Angle {/Symbol W} (mrad)'\n" >> ${GNUPLOT_FILE_AN}
 printf "set ylabel 'dN/d{/Symbol W} (mrad^{-1})'\n" >> ${GNUPLOT_FILE_AN}
 printf "set format y '10^{%%L}'\n" >> ${GNUPLOT_FILE_AN}
 printf "set xrange[%s*1000:%s*1000]\n" "${ANMIN}" "${ANMAX}" >> ${GNUPLOT_FILE_AN}
 printf "set logscale y\n" >> ${GNUPLOT_FILE_AN}
 printf "plot FILE_IN_1 u ((\$1+\$2)*500):(\$3*%s*%s*%s) w histeps lt 1 lc rgb 'blue' lw 3 t 'Angle xz spectrum',\\" "${WEIGHT1}" "${WEIGHT2}" "${RESAMPLING_FACTOR}" >> ${GNUPLOT_FILE_AN}
 printf "\n" >> ${GNUPLOT_FILE_AN}
 printf "FILE_IN_2 u ((\$1+\$2)*500):(\$3*%s*%s*%s) w histeps lt 1 lc rgb 'dark-green' lw 3 t 'Angle yz spectrum' \n" "${WEIGHT1}" "${WEIGHT2}" "${RESAMPLING_FACTOR}" >> ${GNUPLOT_FILE_AN}
 if (( CUT_EN_REQUIRED ))
 then
  GNUPLOT_FILE_AN="spettroAngolare_cut${ECUT}MeV.plt"
  rm -f ${GNUPLOT_FILE_AN}
  touch ${GNUPLOT_FILE_AN}
  chmod 775 ${GNUPLOT_FILE_AN}
  printf "#!/gnuplot\n" >> ${GNUPLOT_FILE_AN}
  printf "FILE_IN_1='%s.ANGLE.binnedX-%s-%s.ppg'\n" "${inputDistr2}" "${ANMIN}" "${ANMAX}" >> ${GNUPLOT_FILE_AN}
  printf "FILE_IN_2='%s.ANGLE.binnedY-%s-%s.ppg'\n" "${inputDistr2}" "${ANMIN}" "${ANMAX}" >> ${GNUPLOT_FILE_AN}
  printf "FILE_OUT='%s.ANGULARspectra.pdf'\n" "${inputDistr2}" >> ${GNUPLOT_FILE_AN}
  printf "set terminal pdf enhanced\n" >> ${GNUPLOT_FILE_AN}
  printf "set output FILE_OUT\n" >> ${GNUPLOT_FILE_AN}
  printf "set xlabel 'Angle {/Symbol W} (mrad)'\n" >> ${GNUPLOT_FILE_AN}
  printf "set ylabel 'dN/d{/Symbol W} (mrad^{-1})'\n" >> ${GNUPLOT_FILE_AN}
  printf "set format y '10^{%%L}'\n" >> ${GNUPLOT_FILE_AN}
  printf "set xrange[%s*1000:%s*1000]\n" "${ANMIN}" "${ANMAX}" >> ${GNUPLOT_FILE_AN}
  printf "set logscale y\n" >> ${GNUPLOT_FILE_AN}
  printf "plot FILE_IN_1 u ((\$1+\$2)*500):(\$3*%s*%s*%s) w histeps lt 1 lc rgb 'blue' lw 3 t 'Angle xz spectrum',\\" "${WEIGHT1}" "${WEIGHT2}" "${RESAMPLING_FACTOR}" >> ${GNUPLOT_FILE_AN}
  printf "\n" >> ${GNUPLOT_FILE_AN}
  printf "FILE_IN_2 u ((\$1+\$2)*500):(\$3*%s*%s*%s) w histeps lt 1 lc rgb 'dark-green' lw 3 t 'Angle yz spectrum' \n" "${WEIGHT1}" "${WEIGHT2}" "${RESAMPLING_FACTOR}" >> ${GNUPLOT_FILE_AN}
 fi
fi
