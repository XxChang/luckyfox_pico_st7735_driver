#include <linux/bitops.h>
#include <linux/delay.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <video/mipi_display.h>

#include "fbtft.h"

#define DRVNAME "fb_st7735s"

static int init_display(struct fbtft_par *par)
{
	par->fbtftops.reset(par);

	/* turn off sleep mode */
	write_reg(par, MIPI_DCS_EXIT_SLEEP_MODE);
	mdelay(120);

	write_reg(par, 0x21);
	write_reg(par, 0x21);

	write_reg(par, 0xB1, 0x05, 0x3A, 0x3A);

	write_reg(par, 0xB2, 0x05, 0x3A, 0x3A);

	write_reg(par, 0xB3, 0x05, 0x3A, 0x3A, 0x05, 0x3A, 0x3A);

	write_reg(par, 0xB4, 0x03);

	write_reg(par, 0xC0, 0x62, 0x02, 0x04);

	write_reg(par, 0xC1, 0xC0);

	write_reg(par, 0xC2, 0x0D, 0x00);

	write_reg(par, 0xC3, 0x8D, 0x6A);

	write_reg(par, 0xC4, 0x8D, 0xEE);

	write_reg(par, 0xC5, 0x0E);

	write_reg(par, 0xC5, 0x0E);

	write_reg(par, 0xE0, 0x10, 0x0E, 0x02, 0x03, 0x0E, 0x07, 0x02, 0x07, 0x0A, 0x12, 0x27, 0x37, 0x00, 0x0D, 0x0E, 0x10);

	write_reg(par, 0xE1, 0x10, 0x0E, 0x03, 0x0F, 0x06, 0x02, 0x08, 0x0A, 0x13, 0x26, 0x36, 0x00, 0x0D, 0x0E, 0x10);

	write_reg(par, 0x3A, 0x05);

	write_reg(par, 0x36, 0xA8);

	write_reg(par, 0x29);

	return 0;
}

static void reset(struct fbtft_par *par)
{
	if (!par->gpio.reset)
		return;

	fbtft_par_dbg(DEBUG_RESET, par, "%s()\n", __func__);

	gpiod_set_value_cansleep(par->gpio.reset, 0);
	usleep_range(200000, 240000);
	gpiod_set_value_cansleep(par->gpio.reset, 1);
	msleep(120);
}

static int verify_gpios(struct fbtft_par *par) {
	struct spi_device *spi = par->spi;
	struct device_node *np = par->spi->dev.of_node;
    struct gpio_desc *dc_gpio;
    struct gpio_desc *reset_gpio;
	struct fbtft_platform_data *pdata = par->pdata;
	int i;

	dev_info(par->info->device, "request gpios\r\n");

    dc_gpio = devm_gpiod_get_from_of_node(&spi->dev, np, "dc", 0, GPIOD_OUT_LOW, "st7735s_dc");
    if (IS_ERR(dc_gpio)) {
        dev_err(par->info->device, "Failed to get dc gpio %ld\n", PTR_ERR(dc_gpio));
        return -ENODEV;
    }
    reset_gpio = devm_gpiod_get_from_of_node(&spi->dev, np, "reset", 0, GPIOD_OUT_LOW, "st7735s_reset");
    if (IS_ERR(reset_gpio)) {
        dev_err(par->info->device, "Failed to get reset gpio\n");
        return -ENODEV;
    }

	par->gpio.dc = dc_gpio;
	par->gpio.reset = reset_gpio;

	fbtft_par_dbg(DEBUG_VERIFY_GPIOS, par, "%s()\n", __func__);

	if (pdata->display.buswidth != 9 &&  par->startbyte == 0 &&
	    !par->gpio.dc) {
		dev_err(par->info->device,
			"Missing info about 'dc' gpio. Aborting.\n");
		return -EINVAL;
	}

	if (!par->pdev)
		return 0;

	if (!par->gpio.wr) {
		dev_err(par->info->device, "Missing 'wr' gpio. Aborting.\n");
		return -EINVAL;
	}
	for (i = 0; i < pdata->display.buswidth; i++) {
		if (!par->gpio.db[i]) {
			dev_err(par->info->device,
				"Missing 'db%02d' gpio. Aborting.\n", i);
			return -EINVAL;
		}
	}
	return 0;
}

static void set_addr_win(struct fbtft_par *par, int xs, int ys, int xe, int ye)
{
	write_reg(par, MIPI_DCS_SET_COLUMN_ADDRESS,
		  xs >> 8, xs & 0xFF, xe >> 8, xe & 0xFF);

	write_reg(par, MIPI_DCS_SET_PAGE_ADDRESS,
		  ys >> 8, ys & 0xFF, ye >> 8, ye & 0xFF);

	write_reg(par, MIPI_DCS_WRITE_MEMORY_START);
}

static struct fbtft_display display = {
	.regwidth = 8,
	.width = 160,
	.height = 80,
	.fbtftops = {
		.init_display = init_display,
		.reset = reset,
		// 内核有bug, request_gpios会被覆盖
		.verify_gpios = verify_gpios,
		.set_addr_win = set_addr_win,
	}
};

FBTFT_REGISTER_DRIVER(DRVNAME, "sitronix,st7735s", &display);

MODULE_DESCRIPTION("FB driver for the ST7735S LCD Controller");
MODULE_AUTHOR("Chang Xu");
MODULE_LICENSE("GPL");
