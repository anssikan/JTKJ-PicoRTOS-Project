// anssi

#include <stdio.h>
#include <string.h>

#include <pico/stdlib.h>

#include <FreeRTOS.h>
#include <queue.h>
#include <task.h>

#include "tkjhat/sdk.h"

// Exercise 4. Include the libraries necessaries to use the usb-serial-debug, and tinyusb

#define DEFAULT_STACK_SIZE 2048
#define CDC_ITF_TX      1

// Exercise 3: Definition of the state machine. Add missing states.
enum state { WAITING=1, DATA_READY};
enum state programState = WAITING;

// Exercise 3: Global variable for ambient light
uint32_t ambientLight;

float ax; 
float ay; 
float az; 
float gx; 
float gy; 
float gz; 
float t;

static void btn_fxn(uint gpio, uint32_t eventMask) {
    // Exercise 1: Toggle the LED. 
    //             Check the SDK and if you do not find a function you would need to implement it yourself. 
    toggle_led();
}

static void sensor_task(void *arg){
    (void)arg;
    // Exercise 2: Init the light sensor. Find in the SDK documentation the adequate function.
    
    // init_veml6030();

    init_ICM42670();
    ICM42670_start_with_default_values();

    for(;;){
                
        // Exercise 2: Modify with application code here. Comment following line.
        //             Read sensor data and print it out as string; 
        // tight_loop_contents(); 


        if (programState == WAITING) {
            ICM42670_read_sensor_data(&ax, &ay, &az, &gx, &gy, &gz, &t);
            programState = DATA_READY;
        }

        // Exercise 3: Modify previous code done for Exercise 2, in previous lines. 
        //             If you are in adequate state, instead of printing save the sensor value 
        //             into the global variable.
        //             After that, modify state
        
        // Exercise 2. Just for sanity check. Please, comment this out
        // printf("sensorTask\n");

        // Do not remove this
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

static void print_task(void *arg){
    (void)arg;
    
    while(1){
        // Exercise 3: Print out sensor data as string to debug window if the state is correct
        //             Remember to modify state
        //             Do not forget to comment next line of code.
        // tight_loop_contents();

        if (programState == DATA_READY) {
            printf("ax: %f ay: %f az: %f gx: %f gy: %f gz: %f \n", ax, ay, az, gx, gy, gz);
            programState = WAITING;
        }
        
        // Exercise 4. Use the usb_serial_print() instead of printf or similar in the previous line.
        //             Check the rest of the code that you do not have printf (substitute them by usb_serial_print())
        //             Use the TinyUSB library to send data through the other serial port (CDC 1).
        //             You can use the functions at https://github.com/hathach/tinyusb/blob/master/src/class/cdc/cdc_device.h
        //             You can find an example at hello_dual_cdc
        //             The data written using this should be provided using csv
        //             timestamp, luminance

        // Exercise 3. Just for sanity check. Please, comment this out
        // printf("printTask\n");
        
        // Do not remove this
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}


// Exercise 4: Uncomment the following line to activate the TinyUSB library.  

/*
static void usbTask(void *arg) {
    (void)arg;
    while (1) {
        tud_task();              // With FreeRTOS wait for events
                                 // Do not add vTaskDelay. 
    }
}*/

int main() {

    // Exercise 4: Comment the statement stdio_init_all(); 
    //             Instead, add AT THE END OF MAIN (before vTaskStartScheduler();) adequate statements to enable the TinyUSB library and the usb-serial-debug.
    //             You can see hello_dual_cdc for help
    //             In CMakeLists.txt add the cfg-dual-usbcdc
    //             In CMakeLists.txt deactivate pico_enable_stdio_usb

    stdio_init_all();

    // Uncomment this lines if you want to wait till the serial monitor is connected
    while (!stdio_usb_connected()){
        sleep_ms(10);
    } 
    
    init_hat_sdk();
    sleep_ms(300); //Wait some time so initialization of USB and hat is done.

    // Exercise 1: Initialize the button and the led and define an register the corresponding interrupton.
    //             Interruption handler is defined up as btn_fxn

    init_button1();
    init_led();
    gpio_set_irq_enabled_with_callback(BUTTON1, GPIO_IRQ_EDGE_FALL, true, btn_fxn);
    
    
    TaskHandle_t hSensorTask, hPrintTask, hUSB = NULL;

    // Exercise 4: Uncomment this xTaskCreate to create the task that enables dual USB communication.

    /*
    xTaskCreate(usbTask, "usb", 2048, NULL, 3, &hUSB);
    #if (configNUMBER_OF_CORES > 1)
        vTaskCoreAffinitySet(hUSB, 1u << 0);
    #endif
    */


    // Create the tasks with xTaskCreate
    BaseType_t result = xTaskCreate(sensor_task, // (en) Task function
                "sensor",                        // (en) Name of the task 
                DEFAULT_STACK_SIZE,              // (en) Size of the stack for this task (in words). Generally 1024 or 2048
                NULL,                            // (en) Arguments of the task 
                2,                               // (en) Priority of this task
                &hSensorTask);                   // (en) A handle to control the execution of this task

    if(result != pdPASS) {
        printf("Sensor task creation failed\n");
        return 0;
    }
    result = xTaskCreate(print_task,  // (en) Task function
                "print",              // (en) Name of the task 
                DEFAULT_STACK_SIZE,   // (en) Size of the stack for this task (in words). Generally 1024 or 2048
                NULL,                 // (en) Arguments of the task 
                2,                    // (en) Priority of this task
                &hPrintTask);         // (en) A handle to control the execution of this task

    if(result != pdPASS) {
        printf("Print Task creation failed\n");
        return 0;
    }

    // Start the scheduler (never returns)
    vTaskStartScheduler();
    
    // Never reach this line.
    return 0;
}

