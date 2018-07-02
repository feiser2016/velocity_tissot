_RELEASE=0

mkzip() {
    [ $_RELEASE -eq 0 ] && cp arch/arm64/boot/Image flasher/Image-custom
    [ $_RELEASE -eq 0 ] && rm -f flasher/.rel
    [ $_RELEASE -eq 1 ] && touch flasher/.rel
    cp arch/arm64/boot/dts/qcom/msm8953-qrd-sku3.dtb flasher/base.dtb
    cp arch/arm64/boot/dts/qcom/msm8953-qrd-sku3-treble.dtb flasher/treble.dtb
    cd flasher

    fn="velocity_kernel.zip"
    [ "x$1" != "x" ] && fn="$1"
    rm -f "../$fn"
    echo "  ZIP     $fn"
    zip -r9 "../$fn" . -x .gitignore > /dev/null
    cd ..
}

rel() {
    _RELEASE=1

    # Swap out version files
    [ ! -f .relversion ] && echo 0 > .relversion
    mv .version .devversion && \
    mv .relversion .version

    # Compile for custom
    make oldconfig && \
    make "${MAKEFLAGS[@]}" -j$jobs && \
    cp arch/arm64/boot/Image flasher/Image-custom && \

    # Reset version
    echo $(($(cat .version) - 1)) >| .version && \

    # Disable pronto for stock
    cp .config .occonfig && \
    sed -i 's/CONFIG_PRONTO_WLAN=y/# CONFIG_PRONTO_WLAN is not set/' .config && \
    make oldconfig && \

    # Compile for stock
    make "${MAKEFLAGS[@]}" -j$jobs && \

    # Create patch delta
    echo '  BSDIFF  flasher/stock.delta' && \
    # Custom bsdiff that matches revised format of flasher patcher
    ./bsdiff flasher/Image-custom arch/arm64/boot/Image flasher/stock.delta

    # Revert version and config files
    mv .occonfig .config && \
    mv .version .relversion && \
    mv .devversion .version

    # Pack zip
    mkdir -p releases && \
    mkzip "releases/velocity_kernel-tissot-r$(cat .relversion)-$(date +%Y%m%d).zip" && \

    # Fix config for next build
    make oldconfig

    _RELEASE=0
}

zerover() {
    echo 0 >| .version
}

cleanbuild() {
    make "${MAKEFLAGS[@]}" clean && make -j$jobs && mkzip
}

incbuild() {
    make "${MAKEFLAGS[@]}" -j$jobs && mkzip
}

dbuild() {
    make "${MAKEFLAGS[@]}" -j$jobs $@ && dzip
}

dzip() {
    mkzip "betas/velocity_kernel-tissot-b$(cat .version)-$(date +%Y%m%d).zip"
}

test() {
    adb wait-for-any && \
    adb shell ls '/init.recovery*' > /dev/null 2>&1
    if [ $? -eq 1 ]; then
        adb reboot recovery
    fi

    fn="velocity_kernel.zip"
    [ "x$1" != "x" ] && fn="$1"
    adb wait-for-usb-recovery && \
    adb push $fn /tmp/kernel.zip && \
    adb shell "twrp install /tmp/kernel.zip && reboot"
}

inc() {
    incbuild && test
}
