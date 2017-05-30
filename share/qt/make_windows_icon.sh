#!/bin/bash
# create multiresolution windows icon
#mainnet
ICON_SRC=../../src/qt/res/icons/credits.png
ICON_DST=../../src/qt/res/icons/credits.ico
convert ${ICON_SRC} -resize 16x16 credits-16.png
convert ${ICON_SRC} -resize 32x32 credits-32.png
convert ${ICON_SRC} -resize 48x48 credits-48.png
convert credits-16.png credits-32.png credits-48.png ${ICON_DST}
#testnet
ICON_SRC=../../src/qt/res/icons/credits_testnet.png
ICON_DST=../../src/qt/res/icons/credits_testnet.ico
convert ${ICON_SRC} -resize 16x16 credits-16.png
convert ${ICON_SRC} -resize 32x32 credits-32.png
convert ${ICON_SRC} -resize 48x48 credits-48.png
convert credits-16.png credits-32.png credits-48.png ${ICON_DST}
rm credits-16.png credits-32.png credits-48.png
