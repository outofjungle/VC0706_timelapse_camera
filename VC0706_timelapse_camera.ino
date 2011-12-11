#include <VC0706.h>
#include <SD.h>
#include <SoftwareSerial.h>

// This is the SD card chip select line, 10 is common
#define chipSelect 4
// This is the camera pin connection. Connect the camera TX
// to pin 2, camera RX to pin 3
SoftwareSerial cameraconnection = SoftwareSerial(2, 3);
// pass the serial connection to the camera object
VC0706 cam = VC0706(&cameraconnection);


void setup() {
    Serial.begin(9600);
    Serial.println("VC0706 Camera test");

    // see if the card is present and can be initialized:
    if (!SD.begin(chipSelect)) {
        Serial.println("Card failed, or not present");
        // don't do anything more:
        return;
    }  

    // Try to locate the camera
    if (cam.begin()) {
        Serial.println("Camera Found:");
    } 
    else {
        Serial.println("No camera found?");
        return;
    }
    // Print out the camera version information (optional)
    char *reply = cam.getVersion();
    if (reply == 0) {
        Serial.print("Failed to get version");
    } 
    else {
        Serial.println("-----------------");
        Serial.print(reply);
        Serial.println("-----------------");
    }

    // Set the picture size - you can choose one of 640x480, 320x240 or 160x120 
    // Remember that bigger pictures take longer to transmit!

    cam.setImageSize(VC0706_640x480);        // biggest
    //cam.setImageSize(VC0706_320x240);        // medium
    //cam.setImageSize(VC0706_160x120);          // small

    // You can read the size back from the camera (optional, but maybe useful?)
    uint8_t imgsize = cam.getImageSize();
    Serial.print("Image size: ");
    if (imgsize == VC0706_640x480) Serial.println("640x480");
    if (imgsize == VC0706_320x240) Serial.println("320x240");
    if (imgsize == VC0706_160x120) Serial.println("160x120");


    //  Motion detection system can alert you when the camera 'sees' motion!
    //cam.setMotionDetect(true);           // turn it on
    cam.setMotionDetect(false);        // turn it off   (default)

    // You can also verify whether motion detection is active!
    Serial.print("Motion detection is ");
    if (cam.getMotionDetect()) 
        Serial.println("ON");
    else 
        Serial.println("OFF");

    delay(3000);
}

void loop() {

    if (! cam.takePicture()) {
        Serial.println("Failed to snap!");
        Serial.println("----------");
    } else {
        Serial.println("Picture taken!");

        // Create an image with the name IMAGExx.JPG
        char filename[13];
        strcpy(filename, "IMAGE00.JPG");
        for (int i = 0; i < 100; i++) {
            filename[5] = '0' + i/10;
            filename[6] = '0' + i%10;
            // create if does not exist, do not open existing, write, sync after write
            if (! SD.exists(filename)) {
                break;
            }
        }

        // Open the file for writing
        File imgFile = SD.open(filename, FILE_WRITE);

        // Get the size of the image (frame) taken  
        uint16_t jpglen = cam.frameLength();
        Serial.print(jpglen, DEC);
        Serial.println(" byte image");

        int32_t time = millis();
        pinMode(8, OUTPUT);
        // Read all the data up to # bytes!
        while (jpglen != 0) {
            // read 64 bytes at a time;
            uint8_t *buffer;
            uint8_t bytesToRead = min(32, jpglen);   // change 32 to 64 for a speedup but may not work with all setups!
            buffer = cam.readPicture(bytesToRead);
            imgFile.write(buffer, bytesToRead);
            //Serial.print("Read ");  Serial.print(bytesToRead, DEC); Serial.println(" bytes");
            jpglen -= bytesToRead;
        }
        imgFile.close();

        time = millis() - time;
        Serial.println("Done!");
        Serial.print("Took "); 
        Serial.print(time); 
        Serial.println(" ms");
        Serial.println("----------");
        cam.resumeVideo();
    }
    delay(100);

}




