/*
Version 1.5
By André Bertelsen
Date: 17/08/2023
*/

// include the library code:
#include <LiquidCrystal.h>
#include <Bounce2.h>

//LCD
LiquidCrystal lcd(13, 12, 10, 9, 8, 7);

//Buttons
const int SW_AirTime1 = 3, SW_pos1 = 5, SW_neg1 = 4, SW_BoutTime = 6, SW_AirTime2 = A0, SW_pos2 = A2, SW_neg2 = A1;

int Bounce_interval = 5;
int cnt = 0;
bool land_flag1 = false;
bool land_flag2 = false;

Bounce2::Button AirTime1 = Bounce2::Button();
Bounce2::Button AirTime2 = Bounce2::Button();
Bounce2::Button pos1 = Bounce2::Button();
Bounce2::Button pos2 = Bounce2::Button();
Bounce2::Button neg1 = Bounce2::Button();
Bounce2::Button neg2 = Bounce2::Button();
Bounce2::Button BoutTime = Bounce2::Button();

//Timer variables
int bout_length = 270; //this is incl 30sec warm up

long int GlobalTime = 0, Player1_AirTime = 0, Player2_AirTime = 0, ground_time1 = 0, ground_time2 = 0, last_millis =0;
int cuts1 = 0, cuts2 = 0, penalty1 = 0, penalty2 = 0, points1 = 0, points2 = 0, point_diff1 = 0,  point_diff2 = 0, s = -30, m = 0;

/*----------------------------------------------------------------------------------------------------*/
void setup() {
  Serial.begin(9600);

  AirTime1.attach(SW_AirTime1, INPUT_PULLUP);
  AirTime2.attach(SW_AirTime2, INPUT_PULLUP);
  pos1.attach(SW_pos1, INPUT_PULLUP);
  pos2.attach(SW_pos2, INPUT_PULLUP);
  neg1.attach(SW_neg1, INPUT_PULLUP);
  neg2.attach(SW_neg2, INPUT_PULLUP);
  BoutTime.attach(SW_BoutTime, INPUT_PULLUP);
  
  // interval in ms
  AirTime1.interval(Bounce_interval);
  AirTime2.interval(Bounce_interval);
  pos1.interval(Bounce_interval);
  pos2.interval(Bounce_interval);
  neg1.interval(Bounce_interval);
  neg2.interval(Bounce_interval);
  BoutTime.interval(Bounce_interval);

  AirTime1.setPressedState(HIGH); 
  AirTime2.setPressedState(HIGH); 
  pos1.setPressedState(LOW); 
  pos2.setPressedState(LOW); 
  neg1.setPressedState(LOW); 
  neg2.setPressedState(LOW); 
  BoutTime.setPressedState(HIGH); 

  // set up the LCD's number of columns and rows:
  lcd.begin(16, 4);

  //init text on display
  init_lcd_text();

  //Timer objects to run
  //timer.setInterval(1000, UpdateTimers);

  Serial.println("Setup done");
}
/*----------------------------------------------------------------------------------------------------*/
void loop() {


  Update_buttons(); //update button states
  GlobalTime2SecMin(); //convert current globaltime to sec and min

  
  if (GlobalTime <= bout_length && BoutTime.read() == HIGH) { //stop timer at 4:00,
    Tick_Tock(); //run timer
    updatecuts_updatepenalty();
    Pointdiff_print();
  }

  if(BoutTime.fell()) {
    Serial.println("Bout end");
    GlobalTime = 0, Player1_AirTime = 0, Player2_AirTime = 0, ground_time1 = 0, ground_time2 = 0;
    cuts1 = 0, cuts2 = 0, penalty1 = 0, penalty2 = 0, points1 = 0, points2 = 0, point_diff1 = 0, point_diff2 = 0;
    s=-30; //a hack to make it work
    m=0;
    land_flag1 = false;
    land_flag2 = false;
    while( BoutTime.read() == LOW){
      Update_buttons(); //keep updating buttons so that we can reset
      }
  }

  if(BoutTime.rose()) {
    Serial.println("Bout Start");
    lcd.clear();
    Tick_Tock();
    updatecuts_updatepenalty();
    init_lcd_text();
    }
}

/*----------------------------------------------------------------------------------------------------*/

void Tick_Tock(){
  long int now = millis();
  
  if(now >= last_millis+1000){ //1000 millis has passed, update timers
    //Serial.println(now - last_millis);     
    GlobalTime++;
    UpdateTimers();
    last_millis = now;


    
  }
}
/*----------------------------------------------------------------------------------------------------*/
void Pointdiff_print(){
  static int point_diff1_old = 0;
  static int point_diff2_old = 0;
  
  point_diff1 =   points1 - ((cuts2 * 100) + ((bout_length - 30) - ground_time2) * 2 - penalty2);
  point_diff2 =   points2 - ((cuts1 * 100) + ((bout_length - 30) - ground_time1) * 2 - penalty1);


  if(ToLandOrNotToLand(points1, points2) && land_flag1 == false){
      land_flag1 = true;
      lcd.setCursor(0, 3);
      lcd.print("LAND");
    }
   else if(land_flag1 == true &&  ToLandOrNotToLand(points1, points2) == false ){//state changed, so delete land flag
      lcd.setCursor(0, 3);
      lcd.print("    ");
      lcd.setCursor(12, 3);
      lcd.print("    ");
      land_flag1 = false;
    }

  if(ToLandOrNotToLand(points2, points1) && land_flag2 == false){
      land_flag2 = true;
      lcd.setCursor(12, 3);
      lcd.print("LAND");
    }
   else if(land_flag2 == true &&  ToLandOrNotToLand(points2, points1) == false ){//state changed, so delete land flag
      lcd.setCursor(0, 3);
      lcd.print("    ");
      lcd.setCursor(12, 3);
      lcd.print("    ");
      land_flag1 = false;
    }


  if(point_diff1 != point_diff1_old  && land_flag1){//points have changed and we can land, print new points

      
      if (point_diff1 > 99) {
        lcd.setCursor(13, 3);
        lcd.print(point_diff1);
      }
      if (point_diff1 < 100) {
        lcd.setCursor(13, 3);
        lcd.print(" ");
        lcd.print(point_diff1);
      }
    }

  if(point_diff2 != point_diff2_old  &&  ToLandOrNotToLand(points2, points1)){//points have changed and we can land, print new points
      lcd.setCursor(12, 3);
      lcd.print("LAND");
      
      if (point_diff2 > 99) {
        lcd.setCursor(0, 3);
        lcd.print(point_diff2);
      }
      if (point_diff2 < 100) {
        lcd.setCursor(0, 3);
        lcd.print(point_diff2);
        lcd.print("  ");
      }
    }


  point_diff1_old = point_diff1;
  point_diff2_old = point_diff2;
  
  }
/*----------------------------------------------------------------------------------------------------*/

void Update_buttons() {
  // Update the Bounce instance :
  AirTime1.update();
  AirTime2.update();
  pos1.update();
  pos2.update();
  neg1.update();
  neg2.update();
  BoutTime.update();
/*
  Serial.print("AirTime1: ");
  Serial.println(AirTime1.read());
  
  Serial.print("AirTime2: ");
  Serial.println(AirTime2.read());
  
  Serial.print("pos1: ");
  Serial.println(pos1.read());
  
  Serial.print("pos2: ");
  Serial.println(pos2.read());
  
  Serial.print("neg1: ");
  Serial.println(neg1.read());
  
  Serial.print("neg2: ");
  Serial.println(neg2.read());
  
  Serial.print("BoutTime: ");
  Serial.println(BoutTime.read());

*/

}

/*----------------------------------------------------------------------------------------------------*/
void GlobalTime2SecMin(){

    s = (GlobalTime-30) % 60;

    m = ((GlobalTime-30)/60) % 60;

    
  if(pos1.read() == LOW &&  pos2.read() == LOW && s < 0 ){ //skip warmup
    while(pos1.read() == LOW &&  pos2.read() == LOW){
      Update_buttons();
      s = 0;
      GlobalTime = 30;
      UpdateTimers();

      }
    
    }

  }

/*----------------------------------------------------------------------------------------------------*/


void UpdateTimers() {

  if (GlobalTime > 31) { //make sure no noe gets airtime in warm-up
    if (AirTime1.read() == HIGH) {
      Player1_AirTime++;
      print_airtime1();
      calc_print_points_1();
    }

    if (AirTime2.read() == HIGH) {
      Player2_AirTime++;
      print_airtime2();
      calc_print_points_2();
    }

    ground_time1 = GlobalTime - Player1_AirTime - 30;
    ground_time2 = GlobalTime - Player2_AirTime - 30;
  }

  if (s < 0) {
    if (s > -10) {
      lcd.setCursor(6, 3);
      lcd.print("-");
      lcd.print(":0");
      lcd.print(s * (-1)); //print it as a positime number
    }
    else {
      lcd.setCursor(6, 3);
      lcd.print("-");
      lcd.print(":");
      lcd.print(s * (-1)); //print it as a positime number
      lcd.print(" ");
    }

  }
  else if (s < 10 && s >= 0) {
    lcd.setCursor(6, 3);
    lcd.print(m);
    lcd.print(":0");
    lcd.print(s);
  }
  else {
    lcd.setCursor(6, 3);
    lcd.print(m);
    lcd.print(":");
    lcd.print(s);
  }
}


/*----------------------------------------------------------------------------------------------------*/

void print_airtime2() {
  if (Player2_AirTime < 10) {
    lcd.setCursor(13, 0);
    lcd.print("00");
    lcd.print(Player2_AirTime);
  }
  if (Player2_AirTime >= 10 &&  Player2_AirTime < 100) {
    lcd.setCursor(13, 0);
    lcd.print("0");
    lcd.print(Player2_AirTime);
  }
  if (Player2_AirTime >= 100) {
    lcd.setCursor(13, 0);
    lcd.print(Player2_AirTime);
  }
}

/*----------------------------------------------------------------------------------------------------*/

void print_airtime1() {
  if (Player1_AirTime < 10) {
    lcd.setCursor(0, 0);
    lcd.print("00");
    lcd.print(Player1_AirTime);
  }
  if (Player1_AirTime >= 10 &&  Player1_AirTime < 100) {
    lcd.setCursor(0, 0);
    lcd.print("0");
    lcd.print(Player1_AirTime);
  }
  if (Player1_AirTime >= 100) {
    lcd.setCursor(0, 0);
    lcd.print(Player1_AirTime);
  }
}

/*----------------------------------------------------------------------------------------------------*/

void updatecuts_updatepenalty() {
  int state_pos1, state_neg1, state_pos2, state_neg2;

  //get the status of the buttons
  state_pos1 = button_state(pos1);
  state_neg1 = button_state(neg1);
  state_pos2 = button_state(pos2);
  state_neg2 = button_state(neg2);

  if (state_pos1 == 1 && GlobalTime > 31) {
    Serial.println("p1: undo cut");
    cuts1 = cuts1 - 1;
    if (cuts1 < 0) {
      cuts1 = 0;
    }
    printcuts_1();
    calc_print_points_1();
  }
  if (state_pos1 == 2 && GlobalTime > 31) { // && state_neg1==-1){
    Serial.println("p1: cut");
    cuts1++;
    printcuts_1();
    calc_print_points_1();
  }
  if (state_neg1 == 1) { // && state_pos1 == -1){
    Serial.println("p1: 100 penalty");
    penalty1 = penalty1 + 100;
    calc_print_points_1();
  }
  if (state_neg1 == 2) { // && state_pos1 == -1){
    Serial.println("p1: 40 penalty");
    penalty1 = penalty1 + 40;
    calc_print_points_1();
  }

  //-----------------------
  if (state_pos2 == 1 && GlobalTime > 31) {
    Serial.println("p2: undo cut");
    cuts2 = cuts2 - 1;
    if (cuts2 < 0) {
      cuts2 = 0;
    }
    printcuts_2();
    calc_print_points_2();
  }
  if (state_pos2 == 2 && GlobalTime > 31) { // && state_neg1==-1){
    Serial.println("p2: cut");
    cuts2++;
    printcuts_2();
    calc_print_points_2();
  }
  if (state_neg2 == 1) { // && state_pos1 == -1){
    Serial.println("p2: 100 penalty");
    penalty2 = penalty2 + 100;
    calc_print_points_2();
  }
  if (state_neg2 == 2) { // && state_pos1 == -1){
    Serial.println("p2: 40 penalty");
    penalty2 = penalty2 + 40;
    calc_print_points_2();
  }



}

/*----------------------------------------------------------------------------------------------------*/
void printcuts_1() {
  static int oldCuts = 0;

  if ((cuts1 == 0 && oldCuts == -1) || (cuts1 == 9 && oldCuts == 10)) { //we overwrite the "stuff" but must delete what is behind
    lcd.setCursor(0, 1);
    lcd.print(cuts1);
    lcd.print(" ");
  }
  else {
    lcd.setCursor(0, 1);
    lcd.print(cuts1);
  }
  oldCuts = cuts1;
}




/*----------------------------------------------------------------------------------------------------*/
void printcuts_2() {

  static int oldCuts = 0;

  if ((cuts2 == 0 && oldCuts == -1) || (cuts2 == 9 && oldCuts == 10)) { //we overwrite the "stuff" but must delete what is behind
    lcd.setCursor(14, 1);
    lcd.print(" ");
    lcd.print(cuts2);
  }
  else {
    if (cuts2 < 0 || cuts2 >= 10) { //make room for the extra
      lcd.setCursor(14, 1);
      lcd.print(cuts2);
    }
    else {
      lcd.setCursor(15, 1);
      lcd.print(cuts2);
    }
  }

  oldCuts = cuts2;
}


/*----------------------------------------------------------------------------------------------------*/

void calc_print_points_1() {

  points1 = (cuts1 * 100) - penalty1 + Player1_AirTime * 2;
  if (points1 < 0) {
    lcd.setCursor(0, 2);
    lcd.print(points1);
    lcd.print(" ");
  }

  if (points1 >= 0 && points1 < 10) {
    lcd.setCursor(0, 2);
    lcd.print(points1);
    lcd.print("   ");
  }
  if (points1 >= 10 &&  points1 < 100) {
    lcd.setCursor(0, 2);
    lcd.print(points1);
    lcd.print(" ");
  }
  if (points1 >= 100) {
    lcd.setCursor(0, 2);
    lcd.print(points1);
  }

}

/*----------------------------------------------------------------------------------------------------*/

void calc_print_points_2() {

  points2 = (cuts2 * 100) - penalty2 + Player2_AirTime * 2;

  if (points2 < 0 && points2 > -10 ) { //from -1 to -9
    lcd.setCursor(12, 2);
    lcd.print("  ");
    lcd.print(points2);
  }
  if (points2 <= -10 && points2 > -100 ) { //from -10 to -99
    lcd.setCursor(12, 2);
    lcd.print(" ");
    lcd.print(points2);
  }
  if (points2 <= -100 ) { //from -100
    lcd.setCursor(12, 2);
    lcd.print(points2);
  }

  if (points2 >= 0 && points2 < 10) { //0 to 9
    lcd.setCursor(12, 2);
    lcd.print("   ");
    lcd.print(points2);
  }
  if (points2 >= 10 &&  points2 < 100) { //10-99
    lcd.setCursor(12, 2);
    lcd.print("  ");
    lcd.print(points2);
  }
  if (points2 >= 100) { //fra 100
    lcd.setCursor(13, 2);
    lcd.print(points2);
  }

}
/*----------------------------------------------------------------------------------------------------*/

boolean ToLandOrNotToLand(int MyPoints, int HisPoints) {

  if (MyPoints > HisPoints + (bout_length - GlobalTime) * 2 + 11) {
    return true;
  }
  else {
    return false;
  }

}

/*----------------------------------------------------------------------------------------------------*
  int points_after_landing(int MyPoints, int HisPoints){
  int point_diff;

  point_diff = MyPoints - (HisPoints + (bount_length-GlobalTime)*2)+11;

  GroundTime = GlobalTime - PlayerX_AirTime




  return point_diff;
  }


  /*----------------------------------------------------------------------------------------------------*/

int button_state(Bounce2::Button &theButton) {

  static long int fellTime = 0;
  static boolean Press = false;

  if (theButton.pressed()) { //see that we pressed the button
    Press = true;      //set a flag that the button is pressed
    fellTime = millis(); //note the time of the press
    Serial.println("a button was pressed");
  }



  if (Press && theButton.rose() &&  millis() - fellTime > 500) { //if the button is high and enough time has passed - long press
    Serial.println("longPress");
    Press = false; //reset flag
    return 1;
  }

  else if (Press && theButton.rose() && millis() - fellTime > Bounce_interval*2 && millis() - fellTime < 500){ //if fell and less that 1000ms has passed - short
    Serial.println("shortPress");
    Press = false; //reset flag
    return 2;
  }
  else {
  // nothing happned
  return -1;
  Serial.println("nothing happned");
  }
}
/*----------------------------------------------------------------------------------------------------*/
void init_lcd_text() {

  lcd.setCursor(0, 0);
  lcd.print("000");
  lcd.print(" ");
  lcd.setCursor(6, 0);
  lcd.print("Time");
  lcd.setCursor(12, 0);
   lcd.print(" ");
  lcd.print("000");

  lcd.setCursor(0, 1);
  lcd.print(cuts1);
  lcd.print("  ");
  lcd.setCursor(6, 1);
  lcd.print("Cuts");
  lcd.setCursor(13, 1);
  lcd.print("  ");
  lcd.print(cuts2);


  lcd.setCursor(0, 2);
  lcd.print(points1);
  lcd.print("   ");
  lcd.setCursor(5, 2);
  lcd.print("Points");
  lcd.setCursor(12, 2);
  lcd.print("   ");
  lcd.print(points2);

  lcd.setCursor(6, 3);
  //lcd.print("-:30");
  
}
