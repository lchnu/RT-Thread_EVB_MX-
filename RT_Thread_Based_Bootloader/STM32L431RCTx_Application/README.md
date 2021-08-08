# RT-Thread Bootloader on Tencent EVB MX+

## 1. RT-Thread OS基本移植

Tencent EVB MX+是腾讯Tencent OS Tiny官方开发板，微控制器采用STM32L431RCT6, 通过QSPI接口连接WinBond W25Q64JV 64Mb (8MB)片外Flash。

### 1.1 新建RT-Thread工程，修改drc_clk.c

1. 新建RT-Thread工程，并且指定合理的文件保存路径、选择芯片、RT-Thread版本号。

注：RT-Thread 4.0.3版本的AT device软件包与UART RX DMA 方式配合不好。在此处我们选择RT-Thread 4.0.2版本。

<img src="https://gitee.com/lchnu/researcher-picture/raw/master/NewProject.png" alt="New RT-Thread Project" style="zoom:50%;" />

2. 在新建项目的图中，我们发现如下语句：`工程使用的是芯片内部HSI时钟，如需修改，请完善drv_clk.c`。

![RT-Thread_CubeMX](https://gitee.com/lchnu/researcher-picture/raw/master/RT-Thread_CubeMX.png)

目前最新RT-Thread Studio 2.1.1版本虽然支持与ST CubeMX的联动，但是，使用CubeMX生成工程后，会出现函数重复定义、头文件不匹配等一系列问题。这些问题可以通过设置RT-Thread项目的头文件目录、添加编译Exclude路径进行解决，但是使用门槛相对较高。

为了保证移植的准确性，降低使用门槛，我们不使用联动。一种可行方案为：在其他目录使用CubeMX新建一个与当前芯片相同的工程，然后从CubeMX生成的工程中复制对应的代码即可。

3. 创建Cube MX工程，配置时钟树，生成工程。

在本示例中，CubeMX工程存储在`D:\RT-Thread\CubeMX\lc.ioc`中。`在CubeMX中将时钟设置为HSE，且使得HCLK为80MHz。`

![RTThread_CubeMX_RCC](https://gitee.com/lchnu/researcher-picture/raw/master/RTThread_CubeMX_RCC.png)

4. 根据CubeMX代码，修改`drv_clk.c`文件

将CubeMX生成的工程中`void SystemClock_Config(void)`函数的部分内容，复制至drv_clk.c文件的`void system_clock_config(int target_freq_mhz)`处。

下方代码块中被屏蔽的部分，是RT-Thread Studio生成的原始代码，37~49行的代码是从CubeMX工程中粘贴而来。

```c
void system_clock_config(int target_freq_mhz)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = { 0 };
    RCC_ClkInitTypeDef RCC_ClkInitStruct = { 0 };

    /** Initializes the CPU, AHB and APB busses clocks
     */
//    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
//    RCC_OscInitStruct.HSIState = RCC_HSI_ON;
//    RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
//    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
//    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
//    RCC_OscInitStruct.PLL.PLLM = 8;
//    RCC_OscInitStruct.PLL.PLLN = target_freq_mhz;
//#if defined(RCC_PLLP_SUPPORT)
//    RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV7;
//#endif
//    RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
//    RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
//    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
//    {
//        Error_Handler();
//    }
//    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
//    RCC_OscInitStruct.HSEState = RCC_HSE_ON;
//    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
//    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
//    RCC_OscInitStruct.PLL.PLLM = 1;
//    RCC_OscInitStruct.PLL.PLLN = 20;
//    RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV7;
//    RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
//    RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
//    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
//    {
//      Error_Handler();
//    }
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLM = 1;
    RCC_OscInitStruct.PLL.PLLN = 20;
    RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV7;
    RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
    RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    {
      Error_Handler();
    }
```

### 1.2 配置RT-Thread工程串口，使能RX DMA


![UART 2 Config (Shell UART)](https://gitee.com/lchnu/researcher-picture/raw/master/UartConfig.png)

![Enanble UART RX DMA](https://gitee.com/lchnu/researcher-picture/raw/master/UARTDMA.png)

在`board.h`中，将UART2部分修改成如下形式：

```c
#define BSP_USING_UART2
#define BSP_UART2_RX_USING_DMA      /**< 添加 UART2 DMA RX*/
#define BSP_UART2_TX_PIN       "PA2"
#define BSP_UART2_RX_PIN       "PA3"
```

![Basic Port OK](https://gitee.com/lchnu/researcher-picture/raw/master/BasicPortOK.png)

## 2. 基于QSPI+W25Q64JV进行FAL移植

### 2.1 EVB MX+硬件信息说明

在Tencent EVB MX+开发板上，W25Q64JV通过QSPI接口与STM32L4进行连接。所以，我们也需要在CubeMX配置QSPI。配置完毕后再次使用CubeMX生成工程。

注意，我们仅仅需要外设的初始化部分。即`HAL_QSPI_MspInit`函数， 用于引脚配置，相关时钟使能, 初始化等操作。

![TencentEVBMX_QSPI1](https://gitee.com/lchnu/researcher-picture/raw/master/TencentEVBMX_QSPI1.png)

![TencentEVBMX_QSPI2](https://gitee.com/lchnu/researcher-picture/raw/master/TencentEVBMX_QSPI2.png)

![RTThread_CubeMX_QSPI](https://gitee.com/lchnu/researcher-picture/raw/master/RTThread_CubeMX_QSPI.png)

### 2.2 配置RT-Thread软件包FAL参数

1. 使能SPI总线，使能SFUD，在SFUD中使用QSPI模式。

![SFUD_QSPI](https://gitee.com/lchnu/researcher-picture/raw/master/SFUD_QSPI.png)



2. 在`board.h`中开启QSPI和On Chip Flash。

在`board.h`中查找下述语句，并取消屏蔽即可。

```c
#define BSP_USING_QSPI
#define BSP_USING_ON_CHIP_FLASH
```

3. 添加FAL软件包，修改FAL设备名称。

![FAL_Config](https://gitee.com/lchnu/researcher-picture/raw/master/FAL_Config.png)

### 2.3 修改FAL分区表

1. 编译工程，会产生11个Errors。提示`fal_cfg.h`头文件没有被包含进工程目录中。

![Errors_1](https://gitee.com/lchnu/researcher-picture/raw/master/Errors_1.png)

2. 按照下图，添加Includes的Path

![RTThread_add_includes](https://gitee.com/lchnu/researcher-picture/raw/master/RTThread_add_includes.png)

3. 再次编译，现在只剩下1个Error。提示`stm32f2_onchip_flash`没有定义.

![Errors_2](https://gitee.com/lchnu/researcher-picture/raw/master/Errors_2.png)

4. 改正`stm32f2_onchip_flash`未定义的错误，并自定义分区表。

打开`packages->fal-v0.5.0->samples->porting`路径下的`fal_cfg.h`，进行修改。

本修改的目的是将`drv_flash_l4.c`中声明的内部FLASH块`stm32_onchip_flash`放入到分区表中，替代FAL例程中不存在的`stm32f2_onchip_flash`。

> 注：
>
> 1. 将`fal_cfg.h`中的`stm32_onchip_flash`修改为`stm32_onchip_flash`。（说明：`stm32_onchip_flash`在`drv_flash_l4.c`文件中定义）
> 3. 将`fal_cfg.h`的`FAL_PART_TABLE`中的字符串`stm32_onchip`修改为`onchip_flash`。（说明：`onchip_flash`是片内FLASH的名称，也在`drv_flash_l4.c`文件中被使用）。
> 4. 将`fal_cfg.h`的`FAL_PART_TABLE`中的字符串`NOR_FLASH_DEV_NAME`修改为`FAL_USING_NOR_FLASH_DEV_NAME`。（说明：`FAL_USING_NOR_FLASH_DEV_NAME`是片外FLASH的名称，在`rtconfig.h`文件中）。

修改完成后的分区表如下所示。

| 分区表      | 偏移  | 大小  | 设备名称                     | 作用                |
| ----------- | ----- | ----- | ---------------------------- | ------------------- |
| bootloader  | 0     | 128KB | on_chip_flash                | Bootloader分区      |
| application | 128KB | 128KB | on_chip_flash                | APP区               |
| download    | 0     | 2MB   | FAL_USING_NOR_FLASH_DEV_NAME | 下载APP固件区       |
| factory     | 2MB   | 2MB   | FAL_USING_NOR_FLASH_DEV_NAME | 出厂APP固件区       |
| easyflash   | 2MB   | 2MB   | FAL_USING_NOR_FLASH_DEV_NAME | 扩展用于EasyFlash区 |
| filesystem  | 2MB   | 2MB   | FAL_USING_NOR_FLASH_DEV_NAME | 扩展用于文件系统区  |



`当然，在实际项目中，具体的偏移量和位置要根据实际情况进行分析和修改。`

![fal_part_table](https://gitee.com/lchnu/researcher-picture/raw/master/fal_part_table.png)



![No Compile Error](https://gitee.com/lchnu/researcher-picture/raw/master/Errors_3.png)


5. 在`board.c`的函数`RT_WEAK void rt_hw_board_init()`下方添加`HAL_QSPI_MspInit`代码。该代码由CubeMX自动生成。
```c
void HAL_QSPI_MspInit(QSPI_HandleTypeDef* qspiHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(qspiHandle->Instance==QUADSPI)
  {
  /* USER CODE BEGIN QUADSPI_MspInit 0 */

  /* USER CODE END QUADSPI_MspInit 0 */
    /* QUADSPI clock enable */
    __HAL_RCC_QSPI_CLK_ENABLE();

    __HAL_RCC_GPIOB_CLK_ENABLE();
    /**QUADSPI GPIO Configuration
    PB0     ------> QUADSPI_BK1_IO1
    PB1     ------> QUADSPI_BK1_IO0
    PB10     ------> QUADSPI_CLK
    PB11     ------> QUADSPI_BK1_NCS
    */
    GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_10|GPIO_PIN_11;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF10_QUADSPI;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* USER CODE BEGIN QUADSPI_MspInit 1 */

  /* USER CODE END QUADSPI_MspInit 1 */
  }
}
```


6. 在终端窗口，输入`list_device`，可以发现，类型为SPI BUS的`qspi1`已经挂载进设备清单。

![RT-Thread_list_device_qspi1](https://gitee.com/lchnu/researcher-picture/raw/master/RT-Thread_list_device_qspi1.png)

此处要特别注意`qspi1`. 在`drv_qspi.c`文件中，`rt_hw_qspi_bus_init`内部使用的总线名称就是`qspi1`, 官方的qspi驱动程序代码如下所示。

```c
static int rt_hw_qspi_bus_init(void)
{
    return stm32_qspi_register_bus(&_stm32_qspi_bus, "qspi1");
}
INIT_BOARD_EXPORT(rt_hw_qspi_bus_init);
```

请不要修改官方的`drv_qspi.c`驱动文件。此处特别指出，是为了帮助理解`2.4 添加QSPI驱动、设备`节中, 将`QSPI_BUS_NAME`定义为`"qspi1"`的原因。


### 2.4 添加QSPI驱动、设备

在`fal_flash_sfud_port.c`文件中，添加如下代码。

> 1. `stm32_qspi_bus_attach_device`函数：将QSPI设备`QSPI_DEVICE_NAME`挂载到名称为`QSPI_BUS_NAME`的QSPI总线上。在RT-Thread中, 所有设备在使用之前都要进行注册，注册后，在使用时，就可以自动检测设备状态，完成自动初始化、打开等之类的工作。
> 2. `rt_sfud_flash_probe`函数：使用 SFUD 探测`QSPI_DEVICE_NAME`从设备， 将 `QSPI_DEVICE_NAME` 连接的 `FAL_USING_NOR_FLASH_DEV_NAME` 初始化为块设备，便于FAL根据Flash Device名称/分区表操作对应的Flash块。
> 3. `INIT_COMPONENT_EXPORT`：将函数导出到自动初始化列表中。

```c
#define QSPI_BUS_NAME       "qspi1"
#define QSPI_DEVICE_NAME    "qspi1_0"
#define QSPI_CS_PIN         GET_PIN(B,11)   //此处注意引脚区分,使用实际电路CS引脚
static int rt_hw_qspi_flash_with_sfud_init(void)
{
    if (stm32_qspi_bus_attach_device(QSPI_BUS_NAME, QSPI_DEVICE_NAME, (rt_uint32_t)QSPI_CS_PIN, 1, RT_NULL, RT_NULL) != RT_EOK)
            return -RT_ERROR;

#ifdef  RT_USING_SFUD
    /**<使用 SFUD 探测  QSPI_DEVICE_NAME 从设备， 将 QSPI_DEVICE_NAME 连接的 FAL_USING_NOR_FLASH_DEV_NAME 初始化为块设备*/
    if (rt_sfud_flash_probe(FAL_USING_NOR_FLASH_DEV_NAME, QSPI_DEVICE_NAME) == RT_NULL)
        return -RT_ERROR;
#endif

    return RT_EOK;
}
INIT_COMPONENT_EXPORT(rt_hw_qspi_flash_with_sfud_init);
```

### 2.5 测试FAL

1. 下载程序，得到下图结果。

![RT-Thread_fal_result](https://gitee.com/lchnu/researcher-picture/raw/master/RT-Thread_fal_result.png)

2. 使用`list_device`命令，得到下图结果。

![RT-Thread_fal_result_1](https://gitee.com/lchnu/researcher-picture/raw/master/RT-Thread_fal_result_1.png)

3. 使用fal系列命令：`fal probe [device_name | part_name]`。

![RT-Thread_fal_result_2](https://gitee.com/lchnu/researcher-picture/raw/master/RT-Thread_fal_result_2.png)

显然，`easyflash`和`W25Q64JV`分别是分区名称和Flash Device名称，所以可以正常Probe。而`qspi1_0`是QSPI设备名称，不能使用FAL Probe命令。请务必掌握它们之间的区别。



4. 使用`fal erase, write, read`等命令
   - 执行了1次读操作，从0x00地址开始读出128字节
   - 执行了1次写操作，写入5个数据到0x00地址
   - 执行了1次读操作，从0x00地址开始读出32字节
   - 执行了1次写操作，写入5个数据到0x10地址
   - 执行了1次读操作，从0x00地址开始读出32字节

![RT-Thread_fal_result_3](https://gitee.com/lchnu/researcher-picture/raw/master/RT-Thread_fal_result_3.png)

`显然，如果再次写入5个数据到0x00地址后，读取出来的数据会有误，写入和读取不相同。`这是由Flash的特性决定的。如果需要更新0x00地址开始的5个数据，则必须需要擦除地址0x00开始的扇区(最小擦除单位)。该操作将删除4096个字节。

> Flash的特性：数据存储的每个位，在更新存储数据时，能把1改成0，而0却不能改为1。所以要求在写入数据前，必须对目标区域进行擦除操作，即把目标区域中的数据位擦除为1。
>
> W25Q64支持扇区擦除、块擦除及整片擦除，最小的擦除单位是扇区，一个块包含16个扇区，有128个块。
>
> 以扇区擦除为例，擦除大小为4KB，即4096字节的数据。擦除实际上是往扇区写入数据，把数据位都写为1。


![RT-Thread_fal_result_4](https://gitee.com/lchnu/researcher-picture/raw/master/RT-Thread_fal_result_4.png)

最后, 我们切换成`bl`分区。 注意，此时分区指向了内部Flash，我们仅仅使用读操作，请务必不要使用写操作和擦除操作。

![RT-Thread_fal_result_5](https://gitee.com/lchnu/researcher-picture/raw/master/RT-Thread_fal_result_5.png)

## 3. Bootloader制作

### 3.1 添加Qboot包支持

按照下图配置qboot软件包

![Qboot Configuration](https://gitee.com/lchnu/researcher-picture/raw/master/qboot_config.png)

### 3.2 Qboot包中配置出厂按钮和LED指示灯

在Tencent EVB MX+电路板上，LED1被配置在PC13，KEY1被配置在PB12，如下图所示

![evb_mx_led_key](https://gitee.com/lchnu/researcher-picture/raw/master/evb_mx_led_key.png)

> 在`drv_gpio.c`中的`static const struct pin_index pins[]`数组中，可以找到LED1和KEY1引脚分别对应45和28。

### 3.3 修改main.c和其他个性化处理

简单修改main.c文件

```c
int main(void)
{
    return RT_EOK;
}

```

在程序修改过程中，我将qboot.c中的所有`“Qboot` 字符串替换成了` "Bootloader`，然后修改了Finsh命令：
```c
/**<原始*/
//MSH_CMD_EXPORT_ALIAS(qbt_shell_cmd, qboot, Quick bootloader test commands); 
/**<修改*/
MSH_CMD_EXPORT_ALIAS(qbt_shell_cmd, bl, Quick bootloader test commands);
```

一切无误后，编译工程，成功得到基于Qboot、FAL的Bootloader程序，占用ROM 114.46KB。如果取消掉Finsh组件，则Bootloader大小会在84KB左右。

![bootloader_rom_size](https://gitee.com/lchnu/researcher-picture/raw/master/bootloader_rom_size.png)

### 3.4 测试

为了确保后续操作无误，使用ST CubeMX Programer将STM32L431的片内Flash进行擦除。擦除后再断开ST Link。

![erase_stm32_flash](https://gitee.com/lchnu/researcher-picture/raw/master/erase_stm32_flash.png)

![bootloader_run1](https://gitee.com/lchnu/researcher-picture/raw/master/bootloader_run1.png)

## 4. Application制作与OTA测试

有了上述基础，按照如下步骤制作Application：

1. 按照第1节"基本移植RT-Thread"进行操作
2. 按照第2节"基于QSPI+W25Q64JV进行FAL移植"进行操作

3. 添加`ota_downloader`软件包。由于STM32L431 Flash的限制，我们仅仅测试Ymodem OTA方式。

![add_ota_downloader](https://gitee.com/lchnu/researcher-picture/raw/master/add_ota_downloader.png)

4. 修改main.c文件

```c
#include <rtthread.h>

#define DBG_TAG "main"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>
#include "board.h"

#include "fal.h"


#define APP_VERSION                      1L              /**< major version number */
#define APP_SUBVERSION                   0L              /**< minor version number */
#define APP_REVISION                     7L              /**< revise version number */

int main(void)
{
    fal_init( ); /*Tang Huimin add comments  for tesing pull request*/

    LOG_I(" Application Software %d.%d.%d build %s\n",
            APP_VERSION, APP_SUBVERSION, APP_REVISION, __DATE__);

    return RT_EOK;
}


/* 将中断向量表起始地址重新设置为 application 分区的起始地址 */
static int rt_hw_app_vector_reconfig(void)
{
    #define NVIC_VECTOR_MASK   0xFFFFFF80
    #define RT_APP_PART_ADDR 0x08020000
    /* 重新设置中断向量表 */
    SCB->VTOR = RT_APP_PART_ADDR & NVIC_VECTOR_MASK;

    return 0;
}
INIT_BOARD_EXPORT(rt_hw_app_vector_reconfig);
```

5. 修改linkscripts

![linkscripts](https://gitee.com/lchnu/researcher-picture/raw/master/linkscripts.png)

6. 编译并下载生成的bin文件，测试程序跳转

将APP下载到Application分区，则Bootloader启动后，判断应用程序存在，会自动跳转至Application。

![bootloader_jump_to_app](https://gitee.com/lchnu/researcher-picture/raw/master/bootloader_jump_to_app.png)

7. 再次修改main.c，编译，`请不要直接下载，我们会使用Ymodem_OTA方式下载`

```c
#define APP_VERSION                      1L              /**< major version number */
#define APP_SUBVERSION                   0L              /**< minor version number */
#define APP_REVISION                     8L              /**< revise version number */
```

8. 制作OTA包。工具位于Bootloader工程的`packages\qboot-v1.05\tools`路径下。

![make_ota_rbl_file](https://gitee.com/lchnu/researcher-picture/raw/master/make_ota_rbl_file.png)

9. 测试Ymodem_ota

![Finsh_help](https://gitee.com/lchnu/researcher-picture/raw/master/Finsh_help.png)

建议使用XShell工具，连接串口。在命令行输入`ymodem_ota`，然后右键选择使用Ymodem方式发送，发送上一步生成的OTA rbl文件，即升级包。
![ymodem_ota](https://gitee.com/lchnu/researcher-picture/raw/master/ymodem_ota.png)

下载完成后，软件重启，首先进入到Bootloader，从download分区搬运代码到application分区（`注意：此处不是简单搬运，而是由qboot代码进行了解压缩后再进行搬运，过程较为复杂。若感兴趣，可进一步深挖quicklz等解压缩软件包。`）

程序升级结果如下，自动从1.07版本升级到1.08版本
![](https://gitee.com/lchnu/researcher-picture/raw/master/ymodem_ota_result.png)


## 5. 填坑

1. RT_APP_PART_ADDR

在`qboot.h`中有如下一段代码：

```c
#ifdef  RT_APP_PART_ADDR
#define QBOOT_APP_ADDR                  RT_APP_PART_ADDR
#else
#define QBOOT_APP_ADDR                  0x08020000
#endif
```

由于在qboot软件包中，并没有配置`RT_APP_PART_ADDR`项。qboot默认APP从内部Flash 128KB处开始。`此处不注意的话，会导致修改内部Flash分区表后，APP无法有效跳转。`

> 如果，我们划分90KB给bootloader，166KB给application，则需要在`fal_cfg.h`文件中修改bootloader、application分区的offset和len。此时，application的中断向量表将会放置在0x08016800地址。为了保证bootloader工作正常，我们需要在bootloader工程的`rtconfig.h`中添加如下代码，否则，bootloader会自动跳转至无效的`QBOOT_APP_ADDR`地址。

```c
#define RT_APP_PART_ADDR 0x08016800 /**<按需要修改成你的application分区偏移*/
```

