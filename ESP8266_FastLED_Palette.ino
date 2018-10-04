/* Garrett Durland
   https://www.youtube.com/watch?time_continue=2&v=3zQ8qC1KQgU
 
   Additions:
   - Added SPEED slider
   - Took out long variable names [FUNTCION1ON] are now [F_1] to avoid serial character overflow
   - Cleaned up the code for better readability
   - Gyro Gearloose, Feb. 29, 2016
 
*/
#include "FastLED.h"
FASTLED_USING_NAMESPACE
 
#if defined(FASTLED_VERSION) && (FASTLED_VERSION < 3001000)
#warning "Requires FastLED 3.1 or later; check github for latest code."
#endif
 
#define LED_PIN     4
//#define CLOCK_PIN   5
//#define LED_TYPE    APA102
//#define COLOR_ORDER BGR
#define LED_TYPE    WS2812B
#define COLOR_ORDER GRB
#define MILLI_AMPERE      2000    // IMPORTANT: set here the max milli-Amps of your power supply 5V 2A = 2000
int FRAMES_PER_SECOND   = 100;    // here you can control the speed.
int ledMode = 2;                  // this is the starting palette
const uint8_t kMatrixWidth  = 16;  
const uint8_t kMatrixHeight = 16;  
const bool    kMatrixSerpentineLayout = true;
int BRIGHTNESS =           128;   // this is half brightness
int new_BRIGHTNESS =       128;   // shall be initially the same as brightness
int SPEED          =       100;   // around half speed
int new_SPEED      =       100;
 
//--Test SmartMatrix stuff------------------------------
#define MATRIX_HEIGHT kMatrixHeight
#define MATRIX_WIDTH kMatrixWidth
const int MATRIX_CENTER_X = MATRIX_WIDTH / 2;
const int MATRIX_CENTER_Y = MATRIX_HEIGHT / 2;
const byte MATRIX_CENTRE_X = MATRIX_CENTER_X - 1;
const byte MATRIX_CENTRE_Y = MATRIX_CENTER_Y - 1;
const uint16_t NUM_LEDS = MATRIX_WIDTH * MATRIX_HEIGHT;
uint16_t XY(uint8_t x, uint8_t y);
 
//added sm int's for overlay effects
int oe1 = 0;
int oe2 = 0;
 
 
//--------------------------
 
#include <ESP8266WiFi.h>
// comes with Huzzah installation. Enter in Arduino settings:
// http://arduino.esp8266.com/package_esp8266com_index.json
 
//!!!!!!!!!!!!!!!!!
//#define USE_AP //   <- select here by commenting/uncommenting
//!!!!!!!!!!!!!!!!!
 
#ifdef USE_AP
 const char* ssid = "LED_Matrix_@3.1";
 const char* password = "12345678";  // set to "" for open access point w/o password; or any other pw (min length = 8 characters)
 
#else
 const char* ssid = "SSID";
 const char* password = "password";
#endif
 
unsigned long ulReqcount;
unsigned long ulReconncount;
 
WiFiServer server(80);  // Create an instance of the server on Port 80
 
// This example combines two features of FastLED to produce a remarkable range of
// effects from a relatively small amount of code.  This example combines FastLED's
// color palette lookup functions with FastLED's Perlin/simplex noise generator, and
// the combination is extremely powerful.
//
// You might want to look at the "ColorPalette" and "Noise" examples separately
// if this example code seems daunting.
//
//
// The basic setup here is that for each frame, we generate a new array of
// 'noise' data, and then map it onto the LED matrix through a color palette.
//
// Periodically, the color palette is changed, and new noise-generation parameters
// are chosen at the same time.  In this example, specific noise-generation
// values have been selected to match the given color palettes; some are faster,
// or slower, or larger, or smaller than others, but there's no reason these
// parameters can't be freely mixed-and-matched.
//
// In addition, this example includes some fast automatic 'data smoothing' at
// lower noise speeds to help produce smoother animations in those cases.
//
// The FastLED built-in color palettes (Forest, Clouds, Lava, Ocean, Party) are
// used, as well as some 'hand-defined' ones, and some proceedurally generated
// palettes.
 
 
#define NUM_LEDS (kMatrixWidth * kMatrixHeight)
#define MAX_DIMENSION ((kMatrixWidth>kMatrixHeight) ? kMatrixWidth : kMatrixHeight)
 
// The leds
CRGB leds[kMatrixWidth * kMatrixHeight];
 
// The 16 bit version of our coordinates
static uint16_t x;
static uint16_t y;
static uint16_t z;
 
// We're using the x/y dimensions to map to the x/y pixels on the matrix.  We'll
// use the z-axis for "time".  speed determines how fast time moves forward.  Try
// 1 for a very slow moving effect, or 60 for something that ends up looking like
// water.
uint16_t speed = 20; // speed is set dynamically once we've started up
 
// Scale determines how far apart the pixels in our noise matrix are.  Try
// changing these values around to see how it affects the motion of the display.  The
// higher the value of scale, the more "zoomed out" the noise iwll be.  A value
// of 1 will be so zoomed in, you'll mostly see solid colors.
uint16_t scale = 30; // scale is set dynamically once we've started up
 
// This is the array that we keep our computed noise values in
uint8_t noise[MAX_DIMENSION][MAX_DIMENSION];
 CRGBPalette16 currentPalette( CRGB::Black );
 
 CRGBPalette16 targetPalette( CRGB::Black );
uint8_t       colorLoop = 1;
 
void setup() {
  ulReqcount=0;         // setup globals for Webserver
  ulReconncount=0;
  // Initialize our coordinates to some random values
  x = random16();
  y = random16();
  z = random16();
  Serial.begin(9600);
  delay(1);
  // inital connect
 
#ifdef USE_AP
  // AP mode
  WiFi.mode(WIFI_AP);
  IPAddress local_ip(192,168,3,1);
  IPAddress gateway(192,168,3,1);
  IPAddress subnet(255,255,255,0);
  WiFi.softAPConfig(local_ip, gateway, subnet);
 
  WiFi.softAP(ssid, password);
  server.begin();
// end ACCESS-Point setup ---------------------------------------------------------------------------------------------------
 
#else
 
// WEB SERVER setup ---------------------------------------------------------------------------------------------------------
  // inital connect
  WiFi.mode(WIFI_STA);
  WiFiStart();
// end WEB SERVER setup -----------------------------------------------------------------------------------------------------
#endif
 
delay(1000); //safety delay
  LEDS.addLeds<LED_TYPE,LED_PIN,COLOR_ORDER>(leds,NUM_LEDS).setCorrection(DirectSunlight);
  //FastLED.addLeds<LED_TYPE,DATA_PIN,CLK_PIN,COLOR_ORDER>(leds, LED_COUNT).setCorrection(DirectSunlight);
  FastLED.setBrightness(BRIGHTNESS);
  set_max_power_in_volts_and_milliamps(5, MILLI_AMPERE);
}
 
void WiFiStart()
{
 
  // Connect to WiFi network
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
 
  WiFi.begin(ssid, password);
 
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
 
  // Start the server
  server.begin();
  Serial.println("Server started");
 
  // Print the IP address
  Serial.println(WiFi.localIP());
}
 
// Fill the x/y array of 8-bit noise values using the inoise8 function.
void fillnoise8() {
  // If we're runing at a low "speed", some 8-bit artifacts become visible
  // from frame-to-frame.  In order to reduce this, we can do some fast data-smoothing.
  // The amount of data smoothing we're doing depends on "speed".
  uint8_t dataSmoothing = 0;
  if( speed < 50) {
    dataSmoothing = 200 - (speed * 4);
  }
 
  for(int i = 0; i < MAX_DIMENSION; i++) {
    int ioffset = scale * i;
    for(int j = 0; j < MAX_DIMENSION; j++) {
      int joffset = scale * j;
     
      uint8_t data = inoise8(x + ioffset,y + joffset,z);
 
      // The range of the inoise8 function is roughly 16-238.
      // These two operations expand those values out to roughly 0..255
      // You can comment them out if you want the raw noise data.
      data = qsub8(data,16);
      data = qadd8(data,scale8(data,39));
 
      if( dataSmoothing ) {
        uint8_t olddata = noise[i][j];
        uint8_t newdata = scale8( olddata, dataSmoothing) + scale8( data, 256 - dataSmoothing);
        data = newdata;
      }
     
      noise[i][j] = data;
    }
  }
 
  z += speed;
 
  // apply slow drift to X and Y, just for visual variation.
  x += speed / 8;
  y -= speed / 16;
}
 
void mapNoiseToLEDsUsingPalette()
{
  static uint8_t ihue=0;
 
  for(int i = 0; i < kMatrixWidth; i++) {
    for(int j = 0; j < kMatrixHeight; j++) {
      // We use the value at the (i,j) coordinate in the noise
      // array for our brightness, and the flipped value from (j,i)
      // for our pixel's index into the color palette.
 
      uint8_t index = noise[j][i];
      uint8_t bri =   noise[i][j];
 
      // if this palette is a 'loop', add a slowly-changing base value
      if( colorLoop) {
        index += ihue;
      }
 
      // brighten up, as the color palette itself often contains the
      // light/dark dynamic range desired
      if( bri > 127 ) {
        bri = 255;
      } else {
        bri = dim8_raw( bri * 2);
      }
 
      CRGB color = ColorFromPalette( currentPalette, index, bri);
      leds[XY(i,j)] = color;
    }
  }
 
  ihue+=1;
}
 
 
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void loop() {
    webserver();
  // Periodically choose a new palette, speed, and scale
  ChangePaletteAndSettingsPeriodically();
      // Crossfade current palette slowly toward the target palette
    //
    // Each time that nblendPaletteTowardPalette is called, small changes
    // are made to currentPalette to bring it closer to matching targetPalette.
    // You can control how many changes are made in each call:
    // - the default of 24 is a good balance
    // - meaningful values are 1-48. 1=veeeeeeeery slow, 48=quickest
    // - "0" means do not change the currentPalette at all; freeze
  uint8_t maxChanges = 24;
  nblendPaletteTowardPalette( currentPalette, targetPalette, maxChanges);
 
  // generate noise data
  fillnoise8();
 
  // convert the noise data to colors in the LED array
  // using the current palette
  mapNoiseToLEDsUsingPalette();
if (oe1 == 1){
  Caleidoscope1();
}
if (oe2 == 1){
  addGlitter(50);
}
 
  show_at_max_brightness_for_power();
 
 
  delay_at_max_brightness_for_power(1000/FRAMES_PER_SECOND);
}
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
 
void webserver() {   /// complete web server (same for access point) ////////////////////////////////////////////////////////
  // Check if a client has connected
  WiFiClient client = server.available();
  if (!client)
  {
    return;
  }
 
  // Wait until the client sends some data
  Serial.println("new client");
  unsigned long ultimeout = millis()+250;
  while(!client.available() && (millis()<ultimeout) )
  {
    delay(1);
  }
  if(millis()>ultimeout)
  {
    Serial.println("client connection time-out!");
    return;
  }
 
  // Read the first line of the request
 
  String sRequest = client.readStringUntil('\r');
  Serial.println(sRequest);
  client.flush();
 
  // stop client, if request is empty
  if(sRequest=="")
  {
    Serial.println("empty request! - stopping client");
    client.stop();
    return;
  }
 
  // get path; end of path is either space or ?
  // Syntax is e.g. GET /?pin=MOTOR1STOP HTTP/1.1
  String sPath="",sParam="", sCmd="";
  String sGetstart="GET ";
  int iStart,iEndSpace,iEndQuest;
  iStart = sRequest.indexOf(sGetstart);
 
 
  if (iStart>=0)
  {
    iStart+=+sGetstart.length();
    iEndSpace = sRequest.indexOf(" ",iStart);
    iEndQuest = sRequest.indexOf("?",iStart);
 
    // are there parameters?
    if(iEndSpace>0)
    {
      if(iEndQuest>0)
      {
        // there are parameters
        sPath  = sRequest.substring(iStart,iEndQuest);
        sParam = sRequest.substring(iEndQuest,iEndSpace);
      }
      else
      {
        // NO parameters
        sPath  = sRequest.substring(iStart,iEndSpace);
      }
    }
  }
 
  ///////////////////////////////////////////////////////////////////////////////
  // output parameters to serial, you may connect e.g. an Arduino and react on it
  ///////////////////////////////////////////////////////////////////////////////
    if(sParam.length()>0)
  { // output parameters
    int iEqu=sParam.indexOf("=");
    if(iEqu>=0)
    {  // is there a message?
      sCmd = sParam.substring(iEqu+1,sParam.length());
//      Serial.print("We are in output Parameters, value is: ");
 
      int iEqu_bright=sParam.indexOf("200");
      if (iEqu_bright>=0)
       {
        sCmd = sParam.substring(iEqu+1,sParam.length());         // BRIGHTNESS
      Serial.println(sCmd);
      char carray[4];                                // values 0..255 = 3 digits; array = digits + 1
      sCmd.toCharArray(carray, sizeof(carray));      // convert char to the array
      new_BRIGHTNESS = atoi(carray);                 // atoi() converts an ascii character array to an integer
      if (new_BRIGHTNESS == 0) {new_BRIGHTNESS = BRIGHTNESS; }
      BRIGHTNESS = new_BRIGHTNESS;
         FastLED.setBrightness(new_BRIGHTNESS);
      Serial.print("new Brightness: ");
      Serial.println(new_BRIGHTNESS);
    }
 
 // space for RGB (300, 400, 500)
 
       int iEqu_speed=sParam.indexOf("600");
      if (iEqu_speed>=0)
       {
      sCmd = sParam.substring(iEqu+1,sParam.length());           // speed
//      Serial.print("We are in SPEED, value is: ");
      Serial.println(sCmd);
      char carray[4];                                // values 0..255 = 3 digits; array = digits + 1
      sCmd.toCharArray(carray, sizeof(carray));      // convert char to the array
      new_SPEED = atoi(carray);                      // atoi() converts an ascii character array to an integer
      if (new_SPEED == 0) {new_SPEED = SPEED; }      // if something else is selected (no change in brightness)
      SPEED = new_SPEED;                
      FRAMES_PER_SECOND = SPEED;
//     FastLED.setMaxRefreshRate(new_SPEED);         // thought this would work  :(
 
      Serial.print("new FPS: ");
      Serial.println(SPEED);
 
       }
 
    } // end is there a message?
  }   // end output Parameters  
 
 
  //////////////////////////////
  // format the html response //
  //////////////////////////////
  String sResponse,sHeader;
 
  ///////////////////////////////
  // 404 for non-matching path //
  ///////////////////////////////
  if(sPath!="/")
  {
    sResponse="<html><head><title>404 Not Found</title></head><body><h1>Not Found</h1><p>The requested URL was not found on this server.</p></body></html>";
   
    sHeader  = "HTTP/1.1 404 Not found\r\n";
    sHeader += "Content-Length: ";
    sHeader += sResponse.length();
    sHeader += "\r\n";
    sHeader += "Content-Type: text/html\r\n";
    sHeader += "Connection: close\r\n";
    sHeader += "\r\n";
  }
  //////////////////////////
  // format the html page //
  //////////////////////////
  else
  {
    ulReqcount++;
    sResponse  = "<html><head><title>ESP_FastLED_Access_Point</title></head><body>";
//    sResponse += "<font color=\"#FFFFF0\"><body bgclror=\"#000000\">";  
    sResponse += "<font color=\"#FFFFF0\"><body bgcolor=\"#151B54\">";  
    sResponse += "<FONT SIZE=-1>";
    sResponse += "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=yes\">";
    sResponse += "<h1>ESP FastLED Noise Plus Palette</h1><br>";
    sResponse += " <h2>Overlay Effects</h2>";
 
/*  this creates a list with ON / OFF buttons
    // &nbsp is a non-breaking space; moves next character over
    sResponse += "<p>Caleidoscope &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<a href=\"?pin=FUNCTION1ON\"><button>--ON--</button></a>&nbsp;<a href=\"?pin=FUNCTION1OFF\"><button>--OFF--</button></a><br>";
    */sResponse += "<p>Caleidoscope &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<a href=\"?pin=FUNCTION1ON\"><button>--ON--</button></a>&nbsp;<a href=\"?pin=FUNCTION1OFF\"><button>--OFF--</button></a><br>";
      sResponse += "<p>AddGlitter &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<a href=\"?pin=FUNCTION2ON\"><button>--ON--</button></a>&nbsp;<a href=\"?pin=FUNCTION2OFF\"><button>--OFF--</button></a><br>";
      /*sResponse += "<p>Caleidoscope6 &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<a href=\"?pin=FUNCTION3ON\"><button>--ON--</button></a>&nbsp;<a href=\"?pin=FUNCTION3OFF\"><button>--OFF--</button></a><br>";
    sResponse += "<p>Juggle&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<a href=\"?pin=FUNCTION5ON\"><button>--ON--</button></a>&nbsp;<a href=\"?pin=FUNCTION5OFF\"><button>--OFF--</button></a></p>";
    sResponse += "<p>BPM&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<a href=\"?pin=FUNCTION6ON\"><button>--ON--</button></a>&nbsp;<a href=\"?pin=FUNCTION6OFF\"><button>--OFF--</button></a></p>";
    sResponse += "<p>Function 7&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<a href=\"?pin=FUNCTION7ON\"><button>--ON--</button></a>&nbsp;<a href=\"?pin=FUNCTION7OFF\"><button>--OFF--</button></a></p><br>";
*/
 
//  This is a nice drop down menue
    sResponse += "<FONT SIZE=+1>";
    sResponse += "<form>";
//    sResponse += "Select Animation<br>";
    sResponse += "<p>Select Palette:</p>";
    sResponse += "<select name=\"sCmd\" size=\"10\" >";
    sResponse += "<option value=\"F_0\">All OFF</option>";
    sResponse += "<option value=\"F_1\">2 Random Colors</option>";
    sResponse += "<option value=\"F_2\">3 Random Colors</option>";
    sResponse += "<option value=\"F_3\">Purple & Green</option>";
    sResponse += "<option value=\"F_4\"selected>Black & White</option>";
    sResponse += "<option value=\"F_5\">Forest Colors</option>";
    sResponse += "<option value=\"F_6\">Cloud Colors</option>";
    sResponse += "<option value=\"F_7\">Lava Colors</option>";
    sResponse += "<option value=\"F_8\">Ocean Colors</option>";
    sResponse += "<option value=\"F_9\">Party Colors</option><br>";
    sResponse += "</select>";
    sResponse += "<br><br>";
    sResponse += "<input type= submit value=\"Show the MAGIC !\">";
    sResponse += "</form>";
    sResponse += "<br><br>";
//    sResponse += "<FONT SIZE=-1>";
 
// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Slider BRIGHTNESS        
sResponse += "</p>";
sResponse += "<form action=\"?sCmd\" >";    // ?sCmd forced the '?' at the right spot  
sResponse += "<input type=\"submit\" value=\"SET\">";
sResponse += "&nbsp BRIGHTNESS &nbsp;&nbsp";  // perhaps we can show here the current value
sResponse += round(new_BRIGHTNESS /2.5);    // this is just a scale depending on the max value; round for better readability
sResponse += " %";
sResponse += "<BR>";
sResponse += "<input style=\"width:250px; height:50px\" type=\"range\" name=\"=F_200\" id=\"cmd\" value=\"";   // '=' in front of F__200 forced the = at the right spot
sResponse += BRIGHTNESS;
sResponse += "\" min=10 max=250 step=10 onchange=\"showValue(points)\" />";
sResponse += "</form>";
sResponse += "<p>";
// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Slider SPEED  
sResponse += "</p>";
sResponse += "<form action=\"?sCmd\" >";    // ?sCmd forced the '?' at the right spot  
sResponse += "<input type=\"submit\" value=\"SET\">";
sResponse += "&nbsp <font color=yellow><b>SPEED &nbsp";  // perhaps we can show here the current value
sResponse += round(new_SPEED /2);    // this is just a scale depending on the max value; round for better readability
sResponse += " %";
sResponse += "<BR>";
sResponse += "<input style=\"width:250px; height:50px\" type=\"range\" name=\"=F_600\" id=\"cmd\" value=\"";   // '=' in front of F__200 forced the = at the right spot
sResponse += SPEED;
sResponse += "\" min=20 max=400 step=20 onchange=\"showValue(points)\" />";
sResponse += "</font></b>";
sResponse += "</form>";
sResponse += "<p>";
// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 
 sResponse += "<FONT SIZE=-1>";
 
    //////////////////////
    // react on parameters
    //////////////////////
    if (sCmd.length()>0)
    {
      if(sCmd.indexOf("F_0")>=0)  { ledMode =  0; }
      if(sCmd.indexOf("F_1")>=0)  { ledMode =  1; }
      if(sCmd.indexOf("F_2")>=0)  { ledMode =  2; }
      if(sCmd.indexOf("F_3")>=0)  { ledMode =  3; }
      if(sCmd.indexOf("F_4")>=0)  { ledMode =  4; }
      if(sCmd.indexOf("F_5")>=0)  { ledMode =  5; }
      if(sCmd.indexOf("F_6")>=0)  { ledMode =  6; }
      if(sCmd.indexOf("F_7")>=0)  { ledMode =  7; }
      if(sCmd.indexOf("F_8")>=0)  { ledMode =  8; }
      if(sCmd.indexOf("F_9")>=0)  { ledMode =  9; }
      if(sCmd.indexOf("FUNCTION1ON")>=0)  { ledMode =  10; }
      if(sCmd.indexOf("FUNCTION1OFF")>=0)  { ledMode =  11; }
      if(sCmd.indexOf("FUNCTION2ON")>=0)  { ledMode =  12; }
      if(sCmd.indexOf("FUNCTION2OFF")>=0)  { ledMode =  13; }
     
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// oh well, Gyro Gearloose was a bit frustrated. He came up with the
// idea to make 10 digits increments and let the URL (not) react on it.
// However, he was able to assign a new_BRIGHTNESS value;
// what after all serves the purpose. Maybe someone comes up with
// a more ellegant way - HOPEFULLY
 
// do not call a new page when the slider is moved, but assign the new value
// to BRIGHTNESS (this is done in "output parameters to serial", line 314
 
      if(sCmd.indexOf("F_200=20")>=0)  { }
      if(sCmd.indexOf("F_200=30")>=0)  { }
      if(sCmd.indexOf("F_200=40")>=0)  { }
      if(sCmd.indexOf("F_200=50")>=0)  { }
      if(sCmd.indexOf("F_200=60")>=0)  { }
      if(sCmd.indexOf("F_200=70")>=0)  { }
      if(sCmd.indexOf("F_200=80")>=0)  { }
      if(sCmd.indexOf("F_200=90")>=0)  { }
      if(sCmd.indexOf("F_200=100")>=0) { }
      if(sCmd.indexOf("F_200=110")>=0) { }
      if(sCmd.indexOf("F_200=120")>=0) { }
      if(sCmd.indexOf("F_200=130")>=0) { }
      if(sCmd.indexOf("F_200=140")>=0) { }
      if(sCmd.indexOf("F_200=150")>=0) { }
      if(sCmd.indexOf("F_200=160")>=0) { }
      if(sCmd.indexOf("F_200=170")>=0) { }
      if(sCmd.indexOf("F_200=180")>=0) { }
      if(sCmd.indexOf("F_200=190")>=0) { }
      if(sCmd.indexOf("F_200=200")>=0) { }
      if(sCmd.indexOf("F_200=210")>=0) { }
      if(sCmd.indexOf("F_200=220")>=0) { }
      if(sCmd.indexOf("F_200=230")>=0) { }
      if(sCmd.indexOf("F_200=240")>=0) { }
      if(sCmd.indexOf("F_200=250")>=0) { }
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 
      if(sCmd.indexOf("F_600=20")>=0)  {  }  
      if(sCmd.indexOf("F_600=40")>=0)  {  }
      if(sCmd.indexOf("F_600=60")>=0)  {  }
      if(sCmd.indexOf("F_600=80")>=0)  {  }
      if(sCmd.indexOf("F_600=100")>=0) {  }
      if(sCmd.indexOf("F_600=120")>=0) {  }
      if(sCmd.indexOf("F_600=140")>=0) {  }
      if(sCmd.indexOf("F_600=160")>=0) {  }
      if(sCmd.indexOf("F_600=180")>=0) {  }
      if(sCmd.indexOf("F_600=200")>=0) {  }
      if(sCmd.indexOf("F_600=220")>=0) {  }
      if(sCmd.indexOf("F_600=240")>=0) {  }
      if(sCmd.indexOf("F_600=260")>=0) {  }
      if(sCmd.indexOf("F_600=280")>=0) {  }
      if(sCmd.indexOf("F_600=300")>=0) {  }
      if(sCmd.indexOf("F_600=320")>=0) {  }
      if(sCmd.indexOf("F_600=340")>=0) {  }
      if(sCmd.indexOf("F_600=360")>=0) {  }
      if(sCmd.indexOf("F_600=380")>=0) {  }
      if(sCmd.indexOf("F_600=400")>=0) {  }
 //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 
 
    } // end sCmd.length()>0
   
   
    sResponse += "<BR>";
    sResponse += "<BR>";
    sResponse += "Powered by FastLED<BR><BR>";
    sResponse += "<FONT SIZE=-2>";
    sResponse += "<font color=\"#FFDE00\">";
    sResponse += "Noise Plus Palette by Mark Kriegsman<BR>";
    sResponse += "Webserver by Stefan Thesen<BR>";
    sResponse += "<font color=\"#FFFFF0\">";
    sResponse += "Gyro Gearloose &nbsp;&nbsp;Feb 2016<BR>";
    sResponse += "SigmazGFX &nbsp;&nbsp;Jun 2016<BR>";
    sResponse += "</body></html>";
    sHeader  = "HTTP/1.1 200 OK\r\n";
    sHeader += "Content-Length: ";
    sHeader += sResponse.length();
    sHeader += "\r\n";
    sHeader += "Content-Type: text/html\r\n";
    sHeader += "Connection: close\r\n";
    sHeader += "\r\n";
 
  }  // end Format HTML page
 
  // Send the response to the client
  client.print(sHeader);
  client.print(sResponse);
 
 
  // and stop the client
  client.stop();
  Serial.println("Client disonnected");  
  }  // end of web server
 
 
// There are several different palettes of colors demonstrated here.
//
// FastLED provides several 'preset' palettes: RainbowColors_p, RainbowStripeColors_p,
// OceanColors_p, CloudColors_p, LavaColors_p, ForestColors_p, and PartyColors_p.
//
// Additionally, you can manually define your own color palettes, or you can write
// code that creates color palettes on the fly.
 
// 1 = 5 sec per palette
// 2 = 10 sec per palette
// etc
#define HOLD_PALETTES_X_TIMES_AS_LONG 2
 
void ChangePaletteAndSettingsPeriodically()
{
  uint8_t maxChanges = 10;
  nblendPaletteTowardPalette( currentPalette, targetPalette, maxChanges);
  //uint8_t secondHand = ((millis() / 1000) / HOLD_PALETTES_X_TIMES_AS_LONG) % 60; //not used with webserver
  //static uint8_t lastSecond = 99;                                                 //not used with webserver
 
    if (ledMode != 999) {
      switch (ledMode) {
      case  0: all_off(); break;
      case  1: SetupRandomPalette(); speed = 3; scale = 25; colorLoop = 1; break; //2-color palette
      case  2: SetupRandomPalette_g(); speed = 3; scale = 25; colorLoop = 1; break; //3-color palette
      case  3: SetupPurpleAndGreenPalette(); speed = 6; scale = 20; colorLoop = 1; break;
      case  4: SetupBlackAndWhiteStripedPalette(); speed = 4; scale = 20; colorLoop = 1; ; break;
      case  5: targetPalette = ForestColors_p; speed = 3; scale = 20; colorLoop = 0; break;
      case  6: targetPalette = CloudColors_p; speed =  4; scale = 20; colorLoop = 0; break;
      case  7: targetPalette = LavaColors_p;  speed =  8; scale = 19; colorLoop = 0; break;
      case  8: targetPalette = OceanColors_p; speed = 6; scale = 25; colorLoop = 0;  break;
      case  9: targetPalette = PartyColors_p; speed = 3; scale = 20; colorLoop = 1; break;
      case 10: oe1 = 1; break;
      case 11: oe1 = 0; break;
      case 12: oe2 = 1; break;
      case 13: oe2 = 0; break;
   
      }
  }
}
 
// This function generates a random palette that's a gradient
// between four different colors.  The first is a dim hue, the second is
// a bright hue, the third is a bright pastel, and the last is
// another bright hue.  This gives some visual bright/dark variation
// which is more interesting than just a gradient of different hues.
 
// LED animations ###############################################################################
void all_off() {  fill_solid( targetPalette, 16, CRGB::Black);}
void SetupRandomPalette()//two colors
{
  EVERY_N_MILLISECONDS( 8000 ){ //new random palette every 8 seconds. Might have to wait for the first one to show up
  CRGB black  = CRGB::Black;
  CRGB random1 = CHSV( random8(), 255, 255);
  CRGB random2 = CHSV( random8(), 255, 255);
  targetPalette = CRGBPalette16(
//                      CRGB( random8(), 255, 32),
//                      CHSV( random8(), 255, 255),
                      random1,random1,black, black,
                      random2,random2,black, black,
                      random1,random1,black, black,
                      random2,random2,black, black);
//                      CHSV( random8(), 128, 255),
//                      CHSV( random8(), 255, 255), );
}
}
void SetupRandomPalette_g()//three colors
{
  EVERY_N_MILLISECONDS( 8000 ){ //new random palette every 8 seconds
  CRGB black  = CRGB::Black;
  CRGB random1 = CHSV( random8(), 255, 255);
  CRGB random2 = CHSV( random8(), 200, 100);
  CRGB random3 = CHSV( random8(), 150, 200);
  targetPalette = CRGBPalette16(
//                      CRGB( random8(), 255, 32),
//                      CHSV( random8(), 255, 255),
                      random1,random1,black, black,
                      random2,random2,black, random3,
                      random1,random1,black, black,
                      random2,random2,black, random3);
//                      CHSV( random8(), 128, 255),
//                      CHSV( random8(), 255, 255), );
}
}
// This function sets up a palette of purple and green stripes.
void SetupPurpleAndGreenPalette()
{
  CRGB purple = CHSV( HUE_PURPLE, 255, 255);
  CRGB green  = CHSV( HUE_GREEN, 255, 255);
  CRGB black  = CRGB::Black;
 
  targetPalette = CRGBPalette16(
   green,  green,  black,  black,
   purple, purple, black,  black,
   green,  green,  black,  black,
   purple, purple, black,  black );
}
// This function sets up a palette of black and white stripes,
// using code.  Since the palette is effectively an array of
// sixteen CRGB colors, the various fill_* functions can be used
// to set them up.
void SetupBlackAndWhiteStripedPalette()
{
  // 'black out' all 16 palette entries...
  fill_solid( targetPalette, 16, CRGB::Black);
  // and set every eighth one to white.
  currentPalette[0] = CRGB::White;
 // currentPalette[4] = CRGB::White;
  currentPalette[8] = CRGB::White;
//  currentPalette[12] = CRGB::White;
}
//
// Mark's xy coordinate mapping code.  See the XYMatrix for more information on it.
//
uint16_t XY( uint8_t x, uint8_t y)
{
  uint16_t i;
  if( kMatrixSerpentineLayout == false) {
    i = (y * kMatrixWidth) + x;
  }
  if( kMatrixSerpentineLayout == true) {
    if( y & 0x01) {
      // Odd rows run backwards
      uint8_t reverseX = (kMatrixWidth - 1) - x;
      i = (y * kMatrixWidth) + reverseX;
    } else {
      // Even rows run forwards
      i = (y * kMatrixWidth) + x;
    }
  }
  return i;
}
void FillLEDsFromPaletteColors(uint8_t colorIndex)
{
  uint8_t brightness = 255;
 
  for( int i = 0; i < NUM_LEDS; i++) {
    leds[i] = ColorFromPalette( currentPalette, colorIndex + sin8(i*16), brightness);
    colorIndex += 3;
  }
}
 
void addGlitter( fract8 chanceOfGlitter)
{
  if ( random8() < chanceOfGlitter) {
    leds[ random16(NUM_LEDS) ] += CRGB::White;
  }
}
 
void Caleidoscope1() {
      for (int x = 0; x < MATRIX_CENTER_X; x++) {
        for (int y = 0; y < MATRIX_CENTER_Y; y++) {
          leds[XY(MATRIX_WIDTH - 1 - x, y)] = leds[XY(x, y)];
          leds[XY(MATRIX_WIDTH - 1 - x, MATRIX_HEIGHT - 1 - y)] = leds[XY(x, y)];
          leds[XY(x, MATRIX_HEIGHT - 1 - y)] = leds[XY(x, y)];
        }
      }
    }
