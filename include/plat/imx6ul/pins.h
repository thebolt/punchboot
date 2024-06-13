#ifndef INCLUDE_PLAT_IMX6UL_PINS_H
#define INCLUDE_PLAT_IMX6UL_PINS_H

#define PAD_CTL_HYS                      (1 << 16)
#define PAD_CTL_PUS_47K_UP               (1 << 14)
#define PAD_CTL_PUS_100K_UP              (2 << 14)

#define PAD_CTL_PUE                      (1 << 13)
#define PAD_CTL_PKE                      (1 << 12)
#define PAD_CTL_SPEED_LOW                (1 << 6)
#define PAD_CTL_SPEED_MED                (2 << 6)
#define PAD_CTL_DSE_DISABLE              (0 << 3)
#define PAD_CTL_DSE_240ohm               (1 << 3)
#define PAD_CTL_DSE_120ohm               (2 << 3)
#define PAD_CTL_DSE_80ohm                (3 << 3)
#define PAD_CTL_DSE_60ohm                (4 << 3)
#define PAD_CTL_DSE_48ohm                (5 << 3)
#define PAD_CTL_DSE_40ohm                (6 << 3)
#define PAD_CTL_DSE_34ohm                (7 << 3)
#define PAD_CTL_SRE_FAST                 (1 << 0)
#define PAD_CTL_SRE_SLOW                 (0 << 0)

#define MUX_ALT(x)                       (x & 0xf)

#define SW_MUX_CTL_PAD_BOOT_MODE0        (0x020e0014)
#define SW_MUX_CTL_PAD_BOOT_MODE1        (0x020e0018)
#define SW_MUX_CTL_PAD_SNVS_TAMPER0      (0x020e001c)
#define SW_MUX_CTL_PAD_SNVS_TAMPER1      (0x020e0020)
#define SW_MUX_CTL_PAD_SNVS_TAMPER2      (0x020e0024)
#define SW_MUX_CTL_PAD_SNVS_TAMPER3      (0x020e0028)
#define SW_MUX_CTL_PAD_SNVS_TAMPER4      (0x020e002c)
#define SW_MUX_CTL_PAD_SNVS_TAMPER5      (0x020e0030)
#define SW_MUX_CTL_PAD_SNVS_TAMPER6      (0x020e0034)
#define SW_MUX_CTL_PAD_SNVS_TAMPER7      (0x020e0038)
#define SW_MUX_CTL_PAD_SNVS_TAMPER8      (0x020e003c)
#define SW_MUX_CTL_PAD_SNVS_TAMPER9      (0x020e0040)
#define SW_MUX_CTL_PAD_JTAG_MOD          (0x020e0044)
#define SW_MUX_CTL_PAD_JTAG_TMS          (0x020e0048)
#define SW_MUX_CTL_PAD_JTAG_TDO          (0x020e004c)
#define SW_MUX_CTL_PAD_JTAG_TDI          (0x020e0050)
#define SW_MUX_CTL_PAD_JTAG_TCK          (0x020e0054)
#define SW_MUX_CTL_PAD_JTAG_TRST_B       (0x020e0058)
#define SW_MUX_CTL_PAD_GPIO1_IO00        (0x020e005c)
#define SW_MUX_CTL_PAD_GPIO1_IO01        (0x020e0060)
#define SW_MUX_CTL_PAD_GPIO1_IO02        (0x020e0064)
#define SW_MUX_CTL_PAD_GPIO1_IO03        (0x020e0068)
#define SW_MUX_CTL_PAD_GPIO1_IO04        (0x020e006c)
#define SW_MUX_CTL_PAD_GPIO1_IO05        (0x020e0070)
#define SW_MUX_CTL_PAD_GPIO1_IO06        (0x020e0074)
#define SW_MUX_CTL_PAD_GPIO1_IO07        (0x020e0078)
#define SW_MUX_CTL_PAD_GPIO1_IO08        (0x020e007c)
#define SW_MUX_CTL_PAD_GPIO1_IO09        (0x020e0080)
#define SW_MUX_CTL_PAD_UART1_TX_DATA     (0x020e0084)
#define SW_MUX_CTL_PAD_UART1_RX_DATA     (0x020e0088)
#define SW_MUX_CTL_PAD_UART1_CTS_B       (0x020e008c)
#define SW_MUX_CTL_PAD_UART1_RTS_B       (0x020e0090)
#define SW_MUX_CTL_PAD_UART2_TX_DATA     (0x020e0094)
#define SW_MUX_CTL_PAD_UART2_RX_DATA     (0x020e0098)
#define SW_MUX_CTL_PAD_UART2_CTS_B       (0x020e009c)
#define SW_MUX_CTL_PAD_UART2_RTS_B       (0x020e00a0)
#define SW_MUX_CTL_PAD_UART3_TX_DATA     (0x020e00a4)
#define SW_MUX_CTL_PAD_UART3_RX_DATA     (0x020e00a8)
#define SW_MUX_CTL_PAD_UART3_CTS_B       (0x020e00ac)
#define SW_MUX_CTL_PAD_UART3_RTS_B       (0x020e00b0)
#define SW_MUX_CTL_PAD_UART4_TX_DATA     (0x020e00b4)
#define SW_MUX_CTL_PAD_UART4_RX_DATA     (0x020e00b8)
#define SW_MUX_CTL_PAD_UART5_TX_DATA     (0x020e00bc)
#define SW_MUX_CTL_PAD_UART5_RX_DATA     (0x020e00c0)
#define SW_MUX_CTL_PAD_ENET1_RX_DATA0    (0x020e00c4)
#define SW_MUX_CTL_PAD_ENET1_RX_DATA1    (0x020e00c8)
#define SW_MUX_CTL_PAD_ENET1_RX_EN       (0x020e00cc)
#define SW_MUX_CTL_PAD_ENET1_TX_DATA0    (0x020e00d0)
#define SW_MUX_CTL_PAD_ENET1_TX_DATA1    (0x020e00d4)
#define SW_MUX_CTL_PAD_ENET1_TX_EN       (0x020e00d8)
#define SW_MUX_CTL_PAD_ENET1_TX_CLK      (0x020e00dc)
#define SW_MUX_CTL_PAD_ENET1_RX_ER       (0x020e00e0)
#define SW_MUX_CTL_PAD_ENET2_RX_DATA0    (0x020e00e4)
#define SW_MUX_CTL_PAD_ENET2_RX_DATA1    (0x020e00e8)
#define SW_MUX_CTL_PAD_ENET2_RX_EN       (0x020e00ec)
#define SW_MUX_CTL_PAD_ENET2_TX_DATA0    (0x020e00f0)
#define SW_MUX_CTL_PAD_ENET2_TX_DATA1    (0x020e00f4)
#define SW_MUX_CTL_PAD_ENET2_TX_EN       (0x020e00f8)
#define SW_MUX_CTL_PAD_ENET2_TX_CLK      (0x020e00fc)
#define SW_MUX_CTL_PAD_ENET2_RX_ER       (0x020e0100)
#define SW_MUX_CTL_PAD_LCD_CLK           (0x020e0104)
#define SW_MUX_CTL_PAD_LCD_ENABLE        (0x020e0108)
#define SW_MUX_CTL_PAD_LCD_HSYNC         (0x020e010c)
#define SW_MUX_CTL_PAD_LCD_VSYNC         (0x020e0110)
#define SW_MUX_CTL_PAD_LCD_RESET         (0x020e0114)
#define SW_MUX_CTL_PAD_LCD_DATA00        (0x020e0118)
#define SW_MUX_CTL_PAD_LCD_DATA01        (0x020e011c)
#define SW_MUX_CTL_PAD_LCD_DATA02        (0x020e0120)
#define SW_MUX_CTL_PAD_LCD_DATA03        (0x020e0124)
#define SW_MUX_CTL_PAD_LCD_DATA04        (0x020e0128)
#define SW_MUX_CTL_PAD_LCD_DATA05        (0x020e012c)
#define SW_MUX_CTL_PAD_LCD_DATA06        (0x020e0130)
#define SW_MUX_CTL_PAD_LCD_DATA07        (0x020e0134)
#define SW_MUX_CTL_PAD_LCD_DATA08        (0x020e0138)
#define SW_MUX_CTL_PAD_LCD_DATA09        (0x020e013c)
#define SW_MUX_CTL_PAD_LCD_DATA10        (0x020e0140)
#define SW_MUX_CTL_PAD_LCD_DATA11        (0x020e0144)
#define SW_MUX_CTL_PAD_LCD_DATA12        (0x020e0148)
#define SW_MUX_CTL_PAD_LCD_DATA13        (0x020e014c)
#define SW_MUX_CTL_PAD_LCD_DATA14        (0x020e0150)
#define SW_MUX_CTL_PAD_LCD_DATA15        (0x020e0154)
#define SW_MUX_CTL_PAD_LCD_DATA16        (0x020e0158)
#define SW_MUX_CTL_PAD_LCD_DATA17        (0x020e015c)
#define SW_MUX_CTL_PAD_LCD_DATA18        (0x020e0160)
#define SW_MUX_CTL_PAD_LCD_DATA19        (0x020e0164)
#define SW_MUX_CTL_PAD_LCD_DATA20        (0x020e0168)
#define SW_MUX_CTL_PAD_LCD_DATA21        (0x020e016c)
#define SW_MUX_CTL_PAD_LCD_DATA22        (0x020e0170)
#define SW_MUX_CTL_PAD_LCD_DATA23        (0x020e0174)
#define SW_MUX_CTL_PAD_NAND_RE_B         (0x020e0178)
#define SW_MUX_CTL_PAD_NAND_WE_B         (0x020e017c)
#define SW_MUX_CTL_PAD_NAND_DATA00       (0x020e0180)
#define SW_MUX_CTL_PAD_NAND_DATA01       (0x020e0184)
#define SW_MUX_CTL_PAD_NAND_DATA02       (0x020e0188)
#define SW_MUX_CTL_PAD_NAND_DATA03       (0x020e018c)
#define SW_MUX_CTL_PAD_NAND_DATA04       (0x020e0190)
#define SW_MUX_CTL_PAD_NAND_DATA05       (0x020e0194)
#define SW_MUX_CTL_PAD_NAND_DATA06       (0x020e0198)
#define SW_MUX_CTL_PAD_NAND_DATA07       (0x020e019c)
#define SW_MUX_CTL_PAD_NAND_ALE          (0x020e01a0)
#define SW_MUX_CTL_PAD_NAND_WP_B         (0x020e01a4)
#define SW_MUX_CTL_PAD_NAND_READY_B      (0x020e01a8)
#define SW_MUX_CTL_PAD_NAND_CE0_B        (0x020e01ac)
#define SW_MUX_CTL_PAD_NAND_CE1_B        (0x020e01b0)
#define SW_MUX_CTL_PAD_NAND_CLE          (0x020e01b4)
#define SW_MUX_CTL_PAD_NAND_DQS          (0x020e01b8)
#define SW_MUX_CTL_PAD_SD1_CMD           (0x020e01bc)
#define SW_MUX_CTL_PAD_SD1_CLK           (0x020e01c0)
#define SW_MUX_CTL_PAD_SD1_DATA0         (0x020e01c4)
#define SW_MUX_CTL_PAD_SD1_DATA1         (0x020e01c8)
#define SW_MUX_CTL_PAD_SD1_DATA2         (0x020e01cc)
#define SW_MUX_CTL_PAD_SD1_DATA3         (0x020e01d0)
#define SW_MUX_CTL_PAD_CSI_MCLK          (0x020e01d4)
#define SW_MUX_CTL_PAD_CSI_PIXCLK        (0x020e01d8)
#define SW_MUX_CTL_PAD_CSI_VSYNC         (0x020e01dc)
#define SW_MUX_CTL_PAD_CSI_HSYNC         (0x020e01e0)
#define SW_MUX_CTL_PAD_CSI_DATA00        (0x020e01e4)
#define SW_MUX_CTL_PAD_CSI_DATA01        (0x020e01e8)
#define SW_MUX_CTL_PAD_CSI_DATA02        (0x020e01ec)
#define SW_MUX_CTL_PAD_CSI_DATA03        (0x020e01f0)
#define SW_MUX_CTL_PAD_CSI_DATA04        (0x020e01f4)
#define SW_MUX_CTL_PAD_CSI_DATA05        (0x020e01f8)
#define SW_MUX_CTL_PAD_CSI_DATA06        (0x020e01fc)
#define SW_MUX_CTL_PAD_CSI_DATA07        (0x020e0200)
#define SW_PAD_CTL_PAD_DRAM_ADDR00       (0x020e0204)
#define SW_PAD_CTL_PAD_DRAM_ADDR01       (0x020e0208)
#define SW_PAD_CTL_PAD_DRAM_ADDR02       (0x020e020c)
#define SW_PAD_CTL_PAD_DRAM_ADDR03       (0x020e0210)
#define SW_PAD_CTL_PAD_DRAM_ADDR04       (0x020e0214)
#define SW_PAD_CTL_PAD_DRAM_ADDR05       (0x020e0218)
#define SW_PAD_CTL_PAD_DRAM_ADDR06       (0x020e021c)
#define SW_PAD_CTL_PAD_DRAM_ADDR07       (0x020e0220)
#define SW_PAD_CTL_PAD_DRAM_ADDR08       (0x020e0224)
#define SW_PAD_CTL_PAD_DRAM_ADDR09       (0x020e0228)
#define SW_PAD_CTL_PAD_DRAM_ADDR10       (0x020e022c)
#define SW_PAD_CTL_PAD_DRAM_ADDR11       (0x020e0230)
#define SW_PAD_CTL_PAD_DRAM_ADDR12       (0x020e0234)
#define SW_PAD_CTL_PAD_DRAM_ADDR13       (0x020e0238)
#define SW_PAD_CTL_PAD_DRAM_ADDR14       (0x020e023c)
#define SW_PAD_CTL_PAD_DRAM_ADDR15       (0x020e0240)
#define SW_PAD_CTL_PAD_DRAM_DQM0         (0x020e0244)
#define SW_PAD_CTL_PAD_DRAM_DQM1         (0x020e0248)
#define SW_PAD_CTL_PAD_DRAM_RAS_B        (0x020e024c)
#define SW_PAD_CTL_PAD_DRAM_CAS_B        (0x020e0250)
#define SW_PAD_CTL_PAD_DRAM_CS0_B        (0x020e0254)
#define SW_PAD_CTL_PAD_DRAM_CS1_B        (0x020e0258)
#define SW_PAD_CTL_PAD_DRAM_SDWE_B       (0x020e025c)
#define SW_PAD_CTL_PAD_DRAM_ODT0         (0x020e0260)
#define SW_PAD_CTL_PAD_DRAM_ODT1         (0x020e0264)
#define SW_PAD_CTL_PAD_DRAM_SDBA0        (0x020e0268)
#define SW_PAD_CTL_PAD_DRAM_SDBA1        (0x020e026c)
#define SW_PAD_CTL_PAD_DRAM_SDBA2        (0x020e0270)
#define SW_PAD_CTL_PAD_DRAM_SDCKE0       (0x020e0274)
#define SW_PAD_CTL_PAD_DRAM_SDCKE1       (0x020e0278)
#define SW_PAD_CTL_PAD_DRAM_SDCLK0_P     (0x020e027c)
#define SW_PAD_CTL_PAD_DRAM_SDQS0_P      (0x020e0280)
#define SW_PAD_CTL_PAD_DRAM_SDQS1_P      (0x020e0284)
#define SW_PAD_CTL_PAD_DRAM_RESET        (0x020e0288)
#define SW_PAD_CTL_PAD_TEST_MODE         (0x020e028c)
#define SW_PAD_CTL_PAD_POR_B             (0x020e0290)
#define SW_PAD_CTL_PAD_ONOFF             (0x020e0294)
#define SW_PAD_CTL_PAD_SNVS_PMIC_ON_REQ  (0x020e0298)
#define SW_PAD_CTL_PAD_CCM_PMIC_STBY_REQ (0x020e029c)
#define SW_PAD_CTL_PAD_BOOT_MODE0        (0x020e02a0)
#define SW_PAD_CTL_PAD_BOOT_MODE1        (0x020e02a4)
#define SW_PAD_CTL_PAD_SNVS_TAMPER0      (0x020e02a8)
#define SW_PAD_CTL_PAD_SNVS_TAMPER1      (0x020e02ac)
#define SW_PAD_CTL_PAD_SNVS_TAMPER2      (0x020e02b0)
#define SW_PAD_CTL_PAD_SNVS_TAMPER3      (0x020e02b4)
#define SW_PAD_CTL_PAD_SNVS_TAMPER4      (0x020e02b8)
#define SW_PAD_CTL_PAD_SNVS_TAMPER5      (0x020e02bc)
#define SW_PAD_CTL_PAD_SNVS_TAMPER6      (0x020e02c0)
#define SW_PAD_CTL_PAD_SNVS_TAMPER7      (0x020e02c4)
#define SW_PAD_CTL_PAD_SNVS_TAMPER8      (0x020e02c8)
#define SW_PAD_CTL_PAD_SNVS_TAMPER9      (0x020e02cc)
#define SW_PAD_CTL_PAD_JTAG_MOD          (0x020e02d0)
#define SW_PAD_CTL_PAD_JTAG_TMS          (0x020e02d4)
#define SW_PAD_CTL_PAD_JTAG_TDO          (0x020e02d8)
#define SW_PAD_CTL_PAD_JTAG_TDI          (0x020e02dc)
#define SW_PAD_CTL_PAD_JTAG_TCK          (0x020e02e0)
#define SW_PAD_CTL_PAD_JTAG_TRST_B       (0x020e02e4)
#define SW_PAD_CTL_PAD_GPIO1_IO00        (0x020e02e8)
#define SW_PAD_CTL_PAD_GPIO1_IO01        (0x020e02ec)
#define SW_PAD_CTL_PAD_GPIO1_IO02        (0x020e02f0)
#define SW_PAD_CTL_PAD_GPIO1_IO03        (0x020e02f4)
#define SW_PAD_CTL_PAD_GPIO1_IO04        (0x020e02f8)
#define SW_PAD_CTL_PAD_GPIO1_IO05        (0x020e02fc)
#define SW_PAD_CTL_PAD_GPIO1_IO06        (0x020e0300)
#define SW_PAD_CTL_PAD_GPIO1_IO07        (0x020e0304)
#define SW_PAD_CTL_PAD_GPIO1_IO08        (0x020e0308)
#define SW_PAD_CTL_PAD_GPIO1_IO09        (0x020e030c)
#define SW_PAD_CTL_PAD_UART1_TX_DATA     (0x020e0310)
#define SW_PAD_CTL_PAD_UART1_RX_DATA     (0x020e0314)
#define SW_PAD_CTL_PAD_UART1_CTS_B       (0x020e0318)
#define SW_PAD_CTL_PAD_UART1_RTS_B       (0x020e031c)
#define SW_PAD_CTL_PAD_UART2_TX_DATA     (0x020e0320)
#define SW_PAD_CTL_PAD_UART2_RX_DATA     (0x020e0324)
#define SW_PAD_CTL_PAD_UART2_CTS_B       (0x020e0328)
#define SW_PAD_CTL_PAD_UART2_RTS_B       (0x020e032c)
#define SW_PAD_CTL_PAD_UART3_TX_DATA     (0x020e0330)
#define SW_PAD_CTL_PAD_UART3_RX_DATA     (0x020e0334)
#define SW_PAD_CTL_PAD_UART3_CTS_B       (0x020e0338)
#define SW_PAD_CTL_PAD_UART3_RTS_B       (0x020e033c)
#define SW_PAD_CTL_PAD_UART4_TX_DATA     (0x020e0340)
#define SW_PAD_CTL_PAD_UART4_RX_DATA     (0x020e0344)
#define SW_PAD_CTL_PAD_UART5_TX_DATA     (0x020e0348)
#define SW_PAD_CTL_PAD_UART5_RX_DATA     (0x020e034c)
#define SW_PAD_CTL_PAD_ENET1_RX_DATA0    (0x020e0350)
#define SW_PAD_CTL_PAD_ENET1_RX_DATA1    (0x020e0354)
#define SW_PAD_CTL_PAD_ENET1_RX_EN       (0x020e0358)
#define SW_PAD_CTL_PAD_ENET1_TX_DATA0    (0x020e035c)
#define SW_PAD_CTL_PAD_ENET1_TX_DATA1    (0x020e0360)
#define SW_PAD_CTL_PAD_ENET1_TX_EN       (0x020e0364)
#define SW_PAD_CTL_PAD_ENET1_TX_CLK      (0x020e0368)
#define SW_PAD_CTL_PAD_ENET1_RX_ER       (0x020e036c)
#define SW_PAD_CTL_PAD_ENET2_RX_DATA0    (0x020e0370)
#define SW_PAD_CTL_PAD_ENET2_RX_DATA1    (0x020e0374)
#define SW_PAD_CTL_PAD_ENET2_RX_EN       (0x020e0378)
#define SW_PAD_CTL_PAD_ENET2_TX_DATA0    (0x020e037c)
#define SW_PAD_CTL_PAD_ENET2_TX_DATA1    (0x020e0380)
#define SW_PAD_CTL_PAD_ENET2_TX_EN       (0x020e0384)
#define SW_PAD_CTL_PAD_ENET2_TX_CLK      (0x020e0388)
#define SW_PAD_CTL_PAD_ENET2_RX_ER       (0x020e038c)
#define SW_PAD_CTL_PAD_LCD_CLK           (0x020e0390)
#define SW_PAD_CTL_PAD_LCD_ENABLE        (0x020e0394)
#define SW_PAD_CTL_PAD_LCD_HSYNC         (0x020e0398)
#define SW_PAD_CTL_PAD_LCD_VSYNC         (0x020e039c)
#define SW_PAD_CTL_PAD_LCD_RESET         (0x020e03a0)
#define SW_PAD_CTL_PAD_LCD_DATA00        (0x020e03a4)
#define SW_PAD_CTL_PAD_LCD_DATA01        (0x020e03a8)
#define SW_PAD_CTL_PAD_LCD_DATA02        (0x020e03ac)
#define SW_PAD_CTL_PAD_LCD_DATA03        (0x020e03b0)
#define SW_PAD_CTL_PAD_LCD_DATA04        (0x020e03b4)
#define SW_PAD_CTL_PAD_LCD_DATA05        (0x020e03b8)
#define SW_PAD_CTL_PAD_LCD_DATA06        (0x020e03bc)
#define SW_PAD_CTL_PAD_LCD_DATA07        (0x020e03c0)
#define SW_PAD_CTL_PAD_LCD_DATA08        (0x020e03c4)
#define SW_PAD_CTL_PAD_LCD_DATA09        (0x020e03c8)
#define SW_PAD_CTL_PAD_LCD_DATA10        (0x020e03cc)
#define SW_PAD_CTL_PAD_LCD_DATA11        (0x020e03d0)
#define SW_PAD_CTL_PAD_LCD_DATA12        (0x020e03d4)
#define SW_PAD_CTL_PAD_LCD_DATA13        (0x020e03d8)
#define SW_PAD_CTL_PAD_LCD_DATA14        (0x020e03dc)
#define SW_PAD_CTL_PAD_LCD_DATA15        (0x020e03e0)
#define SW_PAD_CTL_PAD_LCD_DATA16        (0x020e03e4)
#define SW_PAD_CTL_PAD_LCD_DATA17        (0x020e03e8)
#define SW_PAD_CTL_PAD_LCD_DATA18        (0x020e03ec)
#define SW_PAD_CTL_PAD_LCD_DATA19        (0x020e03f0)
#define SW_PAD_CTL_PAD_LCD_DATA20        (0x020e03f4)
#define SW_PAD_CTL_PAD_LCD_DATA21        (0x020e03f8)
#define SW_PAD_CTL_PAD_LCD_DATA22        (0x020e03fc)
#define SW_PAD_CTL_PAD_LCD_DATA23        (0x020e0400)
#define SW_PAD_CTL_PAD_NAND_RE_B         (0x020e0404)
#define SW_PAD_CTL_PAD_NAND_WE_B         (0x020e0408)
#define SW_PAD_CTL_PAD_NAND_DATA00       (0x020e040c)
#define SW_PAD_CTL_PAD_NAND_DATA01       (0x020e0410)
#define SW_PAD_CTL_PAD_NAND_DATA02       (0x020e0414)
#define SW_PAD_CTL_PAD_NAND_DATA03       (0x020e0418)
#define SW_PAD_CTL_PAD_NAND_DATA04       (0x020e041c)
#define SW_PAD_CTL_PAD_NAND_DATA05       (0x020e0420)
#define SW_PAD_CTL_PAD_NAND_DATA06       (0x020e0424)
#define SW_PAD_CTL_PAD_NAND_DATA07       (0x020e0428)
#define SW_PAD_CTL_PAD_NAND_ALE          (0x020e042c)
#define SW_PAD_CTL_PAD_NAND_WP_B         (0x020e0430)
#define SW_PAD_CTL_PAD_NAND_READY_B      (0x020e0434)
#define SW_PAD_CTL_PAD_NAND_CE0_B        (0x020e0438)
#define SW_PAD_CTL_PAD_NAND_CE1_B        (0x020e043c)
#define SW_PAD_CTL_PAD_NAND_CLE          (0x020e0440)
#define SW_PAD_CTL_PAD_NAND_DQS          (0x020e0444)
#define SW_PAD_CTL_PAD_SD1_CMD           (0x020e0448)
#define SW_PAD_CTL_PAD_SD1_CLK           (0x020e044c)
#define SW_PAD_CTL_PAD_SD1_DATA0         (0x020e0450)
#define SW_PAD_CTL_PAD_SD1_DATA1         (0x020e0454)
#define SW_PAD_CTL_PAD_SD1_DATA2         (0x020e0458)
#define SW_PAD_CTL_PAD_SD1_DATA3         (0x020e045c)
#define SW_PAD_CTL_PAD_CSI_MCLK          (0x020e0460)
#define SW_PAD_CTL_PAD_CSI_PIXCLK        (0x020e0464)
#define SW_PAD_CTL_PAD_CSI_VSYNC         (0x020e0468)
#define SW_PAD_CTL_PAD_CSI_HSYNC         (0x020e046c)
#define SW_PAD_CTL_PAD_CSI_DATA00        (0x020e0470)
#define SW_PAD_CTL_PAD_CSI_DATA01        (0x020e0474)
#define SW_PAD_CTL_PAD_CSI_DATA02        (0x020e0478)
#define SW_PAD_CTL_PAD_CSI_DATA03        (0x020e047c)
#define SW_PAD_CTL_PAD_CSI_DATA04        (0x020e0480)
#define SW_PAD_CTL_PAD_CSI_DATA05        (0x020e0484)
#define SW_PAD_CTL_PAD_CSI_DATA06        (0x020e0488)
#define SW_PAD_CTL_PAD_CSI_DATA07        (0x020e048c)

#endif