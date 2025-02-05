#define QUEUE_SIZE 10 

const unsigned long SPEED = 650; // move speed 
const int DELAY = 500; // debounce time
const int BUTTON_1_FLOOR = 11;
const int BUTTON_2_FLOOR = 12;
const int BUTTON_3_FLOOR = 13;
const int STATE_LEDS[7] = {A0, 2, 3, 4, 5, 6, 7};
const int USER_LEDS[3] = {8, 9, 10};

unsigned long last_move_time = 0;
unsigned long last_debounce_time1 = 0;
unsigned long last_debounce_time2 = 0;
unsigned long last_debounce_time3 = 0;
int move_flag = 0;
int current_floor = 0;
int target_floor = 0;

bool visited[3];

class Queue {
private:
  int arr[QUEUE_SIZE];  
  int front, rear, size;
public:
  Queue() {
    front = 0;
    rear = -1;
    size = 0;
  }
  bool isEmpty() { return size == 0; }
  bool isFull() { return size == QUEUE_SIZE; }
  void push(int value) {
    if (!isFull()) {
      rear = (rear + 1) % QUEUE_SIZE;  
      arr[rear] = value;
      size++;
    }
  }
  int front_pop() {
    if (!isEmpty()) {
      int value = arr[front];
      front = (front + 1) % QUEUE_SIZE;  
      size--;
      return value;
    }
    return -1;  
  }
  int count() { return size; }
};

Queue queue;

void setup() {
  Serial.begin(9600);
  for (int i = 0; i < 7; i++) pinMode(STATE_LEDS[i], OUTPUT);
  for (int i = 0; i < 3; i++) pinMode(USER_LEDS[i], OUTPUT);
  pinMode(BUTTON_1_FLOOR, INPUT);
  pinMode(BUTTON_2_FLOOR, INPUT);
  pinMode(BUTTON_3_FLOOR, INPUT);
  current_floor = 0;
  digitalWrite(STATE_LEDS[0], HIGH);
  memset(&visited[0], 0, sizeof(visited));
}

// check button, queue push
void checkButtonPress() {
  unsigned long current_time = millis();
  if (digitalRead(BUTTON_1_FLOOR) == HIGH && (current_time - last_debounce_time1 > DELAY)) {
    digitalWrite(USER_LEDS[0], HIGH);
    queue.push(0);
    if (visited[0] == 1) {
      visited[0] = 0;  
      digitalWrite(USER_LEDS[0], LOW);
    }
    else visited[0] = 1;
    last_debounce_time1 = current_time;
  }
  if (digitalRead(BUTTON_2_FLOOR) == HIGH && (current_time - last_debounce_time2 > DELAY)) {
    digitalWrite(USER_LEDS[1], HIGH);
    queue.push(3);
    if (visited[1] == 1) {
      visited[1] = 0;  
      digitalWrite(USER_LEDS[1], LOW);
    }
    else visited[1] = 1;
    last_debounce_time2 = current_time;
  }
  if (digitalRead(BUTTON_3_FLOOR) == HIGH && (current_time - last_debounce_time3 > DELAY)) {
    digitalWrite(USER_LEDS[2], HIGH);
    queue.push(6);
    if (visited[2] == 1) {
      visited[2] = 0;  
      digitalWrite(USER_LEDS[2], LOW);
    }
    else visited[2] = 1;
    last_debounce_time3 = current_time;
  }
}

// move elevator
void processElevatorMovement() {
  unsigned long current_time = millis();
  
  // stop
  if (!move_flag && !queue.isEmpty()) {
    target_floor = queue.front_pop();
    // target alive
    if (visited[target_floor / 3] != 0) {
      move_flag = true;
      last_move_time = current_time;
    }
  }
  
  // move
  if (move_flag && (current_time - last_move_time >= SPEED)) {
    // target death while moving
    if (!visited[target_floor / 3]) {
      if (move_flag == 1) {
        digitalWrite(STATE_LEDS[++current_floor], HIGH);
        digitalWrite(STATE_LEDS[current_floor - 1], LOW);
        move_flag = 1;
      }
      else if (move_flag == 2) {
        digitalWrite(STATE_LEDS[--current_floor], HIGH);
        digitalWrite(STATE_LEDS[current_floor + 1], LOW);
        move_flag = 2;
      }
      if (current_floor % 3 == 0) {
        target_floor = current_floor;
        move_flag = 0;
      }
    }

    // normal movement UP 
    else if (current_floor < target_floor) { 
      digitalWrite(STATE_LEDS[++current_floor], HIGH);
      digitalWrite(STATE_LEDS[current_floor - 1], LOW);
      // meet visited light while up
      if (current_floor % 3 == 0 && visited[current_floor / 3]) {
        visited[current_floor / 3] = 0;
        digitalWrite(USER_LEDS[current_floor / 3], LOW);
      }
      move_flag = 1;
    } 
    // normal movement DOWN
    else if (current_floor > target_floor) {
      digitalWrite(STATE_LEDS[--current_floor], HIGH);
      digitalWrite(STATE_LEDS[current_floor + 1], LOW);
      // meet visited light while down
      if (current_floor % 3 == 0 && visited[current_floor / 3]) {
        visited[current_floor / 3] = 0;
        digitalWrite(USER_LEDS[current_floor / 3], LOW);
      }
      move_flag = 2;
    }
    
    last_move_time = current_time;  
    
    // arrive
    if (current_floor == target_floor) {
      digitalWrite(USER_LEDS[target_floor / 3], LOW);
      visited[target_floor / 3] = 0;
      move_flag = 0; 
    }
  }
}

void loop() {
  checkButtonPress();
  processElevatorMovement();
}
