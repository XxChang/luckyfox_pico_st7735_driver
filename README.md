# luckyfox_pico_st7735_driver
st7735 driver module for luckyfox_pico board

# How to install
modified device tree
```
&spi0 {
	status = "okay";
	pinctrl-names = "default";
	pinctrl-0 = <&spi0m0_cs0 &spi0m0_pins>;

	st7735s@0 {
		status = "okay";
		compatible = "sitronix,st7735s";
		reg = <0>;
		spi-max-frequency = <20000000>;
		fps = <20>;
        buswidth = <8>;
        debug = <0x7>;
        dc = <&gpio1 RK_PA2 GPIO_ACTIVE_HIGH>;        //DC
        reset = <&gpio1 RK_PC3 GPIO_ACTIVE_HIGH>;     //RES
		bl = <&gpio0 RK_PA4 GPIO_ACTIVE_HIGH>;	    //BLINK
	};
};
```

```
make ARCH=arm CROSS_COMPILE=arm-rockchip830-linux-uclibcgnueabihf-
```

# Test it
```
dd if=/dev/random of=/dev/fb0 bs=1024 count=25
```