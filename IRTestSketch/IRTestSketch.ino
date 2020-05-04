#define IR1 A1
#define IR2 A2
#define IR3 A3

int reading1;
int reading2;
int reading3;

void setup() {
  Serial.begin(9600);
}

void loop() {
  reading1 = analogRead(IR1);
  reading2 = analogRead(IR2);
  reading3 = analogRead(IR3);
  Serial.print("IR1: ");
  Serial.println(reading1);
  Serial.print("IR2: ");
  Serial.println(reading2);
  Serial.print("IR3: ");
  Serial.println(reading3);
  Serial.println("");
  delay(1000);
}
