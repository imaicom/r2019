// cc -o self self.c -lwiringPi -lm
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/joystick.h>

#include <wiringPi.h>
#include <softPwm.h>

#include "controller.h"
#include <math.h>

int write_file(char fnp[256],long int d) {
	
	FILE *fp;
	char fn[256]="/tmp/";

	strcat(fn,fnp);
	strcat(fn,".txt");
	
	if((fp=fopen(fn,"r+"))==NULL) {
		fp=fopen(fn,"w+");
	};
	
	fprintf(fp,"%8d",d);
	fclose(fp);
}

struct ps3ctls {
	
	int fd;
	unsigned char nr_buttons;	// Max number of Buttons
	unsigned char nr_sticks;	// Max number of Sticks
	short *button;			// button[nr_buttons]
	short *stick;			// stick[nr_sticks]
};

// global
int fds;
int s0 = 0;
int s1 = 0;
unsigned int timWheel;
int cntWheel = 0;
char lcd_buff[60] = "";
long int before_bar , before_cntWheel;

int clr_LCD() {
	system("i2cset -y 1 0x50 0x00 0x01"); // Clear Display
	system("i2cset -y 1 0x50 0x00 0x38"); // Function Set 8Bit-Mode , 2Line-Mode
	system("i2cset -y 1 0x50 0x00 0x0c"); // Display On , Cursor Off , Blinking Off
	system("i2cset -y 1 0x50 0x00 0x06"); // Entry Mode Set
}

int set_posLCD(char p) {
	int fd;
	fd = wiringPiI2CSetup(0x50);	// ACM1602NI
	wiringPiI2CWriteReg8(fd , 0x00 , p|0x80);
    nanosleep(10000);
}

int put_LCD(char a) {
	int fd;
	fd = wiringPiI2CSetup(0x50);	// ACM1602NI
	wiringPiI2CWriteReg8(fd , 0x80 , a);
    nanosleep(10000);
}

void put_LCDstring(char *str) {
	while(*str != '\0') put_LCD(*str++);
}

int check_file(char fnp[256]) {
	
	FILE *fp;
	char fn[256]="/tmp/";
	long int d;

	strcat(fn,fnp);
	strcat(fn,".txt");
	
	if((fp=fopen(fn,"r"))!=NULL) {
		fscanf(fp,"%8d",&d);
		fclose(fp);
	} else d=0;
	return d;
}

int check_port() {
	if (timWheel != digitalRead(12)) {
		timWheel = digitalRead(12);
		cntWheel++;
	};
	if(!(digitalRead( 5))) {
		softPwmWrite(16,0); // motor 1 off
		softPwmWrite( 1,0);
		softPwmWrite(28,0); // motor 2 off
		softPwmWrite(29,0);
		softPwmWrite(25,0); // beep off
		sleep(1);
		system("sudo shutdown -h now &");
	};
//	if(digitalRead(13)+digitalRead(14)>0) {
//		softPwmWrite(25,50);
//	} else {
//		softPwmWrite(25,0);
//	};
}

int ps3c_test(struct ps3ctls *ps3dat) {

	unsigned char i;
	unsigned char nr_btn = ps3dat->nr_buttons;
	unsigned char nr_stk = ps3dat->nr_sticks;
	int m1,m2;
	long int temp;

/*
  	printf(" 1=%2d ",ps3dat->button[PAD_KEY_LEFT]);
	printf(" 2=%2d ",ps3dat->button[PAD_KEY_RIGHT]);
	printf(" 3=%2d ",ps3dat->button[PAD_KEY_UP]);
	printf(" 4=%2d ",ps3dat->button[PAD_KEY_DOWN]);
	printf(" 5=%4d ",ps3dat->stick [PAD_LEFT_X]);
	printf(" 6=%4d ",ps3dat->stick [PAD_LEFT_Y]);
	printf(" 7=%4d ",ps3dat->stick [PAD_RIGHT_X]);
	printf(" 8=%4d ",ps3dat->stick [PAD_RIGHT_Y]);
	printf(" 9=%4d ",digitalRead(12));
	printf(" a=%4d ",digitalRead(13));
	printf(" b=%4d ",digitalRead(14));
	printf(" c=%4d ",digitalRead(15));
	printf(" d=%4d ",digitalRead( 3));
	printf(" e=%4d ",check_file("cntWheel"));
	printf(" f=%4d ",digitalRead( 5));
	printf(" g=%4d ",digitalRead( 6));
	*/
	printf(" a=%4d ",digitalRead(21));
	printf(" b=%4d ",digitalRead(22));
	printf(" e=%4d ",digitalRead(30));
	printf(" c=%4d ",digitalRead(23));
	printf(" d=%4d ",digitalRead(24));
	printf("\n"); 

	if (ps3dat->button[PAD_KEY_TRIANGLE]) {
		digitalWrite(7,1);
	} else {
		digitalWrite(7,0);
	};

	if (ps3dat->button[PAD_KEY_CIRCLE]) {
		softPwmWrite(25,50);
	} else {
		softPwmWrite(25,0);
	};
	

	m2 = ps3dat->stick [PAD_LEFT_Y ];
//	m2 = ps3dat->stick [PAD_RIGHT_Y];
	m1 = m2 * 9/10;

	if(abs(m1) < 5) {
		softPwmWrite(16,0);
		softPwmWrite( 1,0);
	} else if(m1 > 0) {
		softPwmWrite(16,0);
		softPwmWrite( 1,abs(m1));
	} else {
		softPwmWrite(16,abs(m1));
		softPwmWrite( 1,0);
	};

	if(abs(m2) < 5) {
		softPwmWrite(28,0);
		softPwmWrite(29,0);
	} else if(m2 > 0) {
		softPwmWrite(28,0);
		softPwmWrite(29,abs(m2));
	} else {
		softPwmWrite(28,abs(m2));
		softPwmWrite(29,0);
	};
	
//	if(ps3dat->button[PAD_KEY_CROSS]==1) return -1; // end of program
	return 0;
}


int ps3c_input(struct ps3ctls *ps3dat) {

	int rp;
	struct js_event ev;

	do {
		rp = read(ps3dat->fd, &ev, sizeof(struct js_event));
		if (rp != sizeof(struct js_event)) {
			return -1;
		}

	} while (ev.type & JS_EVENT_INIT);

	switch (ev.type) {
		case JS_EVENT_BUTTON:
			if (ev.number < ps3dat->nr_buttons) {
				ps3dat->button[ev.number] = ev.value;
			}
			break;
		case JS_EVENT_AXIS:
			if (ev.number < ps3dat->nr_sticks) {
				ps3dat->stick[ev.number] = ev.value / 327; // range -32767 ~ +32768 -> -100 ~ +100
			}
			break;
		default:
			break;
	}

	return 0;
}

int ps3c_getinfo(struct ps3ctls *ps3dat) {

	if(ioctl(ps3dat->fd , JSIOCGBUTTONS , &ps3dat->nr_buttons) < 0) return -1;
	if(ioctl(ps3dat->fd , JSIOCGAXES    , &ps3dat->nr_sticks ) < 0) return -2;

	return 0;
}


int ps3c_init(struct ps3ctls *ps3dat, const char *df) {

	unsigned char nr_btn;
	unsigned char nr_stk;
	unsigned char *p;
	long int i,j;

	clr_LCD();
	set_posLCD(0);
	for (i=0;i<20;i++) {for (j=0;j<5000000;j++);put_LCD(0xff);};
	system("mpg123 /home/pi/Music/all_curcuit_on.mp3");
	clr_LCD();
	set_posLCD(0);
	put_LCDstring("Hello!");
	set_posLCD(0x40);
	put_LCDstring("                ");
	sleep(1);
//	set_posLCD(0);
//	put_LCDstring("                ");
	
	before_bar = check_file("bar");
	before_cntWheel = check_file("cntWheel");
	
	ps3dat->fd = open(df, O_RDONLY);
	if (ps3dat->fd < 0) return -1;

	if (ps3c_getinfo(ps3dat) < 0) {
		close(ps3dat->fd);
		return -2;
	}

	nr_btn = ps3dat->nr_buttons;
	nr_stk = ps3dat->nr_sticks;

	p = calloc(nr_btn + nr_stk , sizeof(short));
	if (p == NULL) {
		close(ps3dat->fd);
		return -3;
	}
	ps3dat->button = (short *)p;
	ps3dat->stick  = (short *)&p[nr_btn * sizeof(short)];
			
	return 0;
}

void ps3c_exit   (struct ps3ctls *ps3dat) {

	free (ps3dat->button);
	close(ps3dat->fd);
}

void main() {

	char *df = "/dev/input/js0";
	struct ps3ctls ps3dat;
	long int temp;
	int before_digital = 0;
	int bmeter = 0;

	wiringPiSetup();
	softPwmCreate(16,0,20); // start-0 10ms
	softPwmCreate( 1,0,20); // start-0 10ms
	softPwmCreate(28,0,20); // start-0 10ms
	softPwmCreate(29,0,20); // start-0 10ms
	softPwmCreate(25,0,20); // start-0 10ms

	pinMode( 3,INPUT);pullUpDnControl( 3,PUD_UP);
	pinMode(12,INPUT);pullUpDnControl(12,PUD_UP);
	pinMode(21,INPUT);pullUpDnControl(21,PUD_UP); // 1 right
	pinMode(22,INPUT);pullUpDnControl(22,PUD_UP); // 2 right-center
	pinMode(30,INPUT);pullUpDnControl(30,PUD_UP); // center
	pinMode(23,INPUT);pullUpDnControl(23,PUD_UP); // 3 left-center
	pinMode(24,INPUT);pullUpDnControl(24,PUD_UP); // 4 left
	pinMode(15,INPUT);pullUpDnControl(15,PUD_UP);
	pinMode( 5,INPUT);pullUpDnControl( 5,PUD_UP);
	pinMode( 6,INPUT);pullUpDnControl( 6,PUD_UP);
	
	pinMode(4,OUTPUT);digitalWrite(4,0);
	pinMode(7,OUTPUT);digitalWrite(7,0);
	write_file("cntWheel"		,0 );
	write_file("bar"			,0 );


//	while(1) {
		if(!(ps3c_init(&ps3dat, df))) {

			do {
				if (ps3c_test(&ps3dat) < 0) break;
				if(digitalRead( 3)) {
					digitalWrite(4,1);
					break;
				};
//				if(before_digital != digitalRead(21)+digitalRead(22)+digitalRead(23)+digitalRead(24)) {
//					before_digital = digitalRead(21)+digitalRead(22)+digitalRead(23)+digitalRead(24);
					set_posLCD(64);
					sprintf(lcd_buff , " %5d",check_file("bar"));
					put_LCDstring(lcd_buff);
					sprintf(lcd_buff , " %8d",check_file("cntWheel"));
					put_LCDstring(lcd_buff);
//				};
				
				if(!digitalRead(5)){
//					before_bar = check_file("bar");
//					before_cntWheel = check_file("cntWheel");
					set_posLCD(64);
					sprintf(lcd_buff , " %5d",check_file("bar"));
					put_LCDstring(lcd_buff);
					sprintf(lcd_buff , " %8d",check_file("cntWheel"));
					put_LCDstring(lcd_buff);
				};
				
				if(digitalRead(22)+digitalRead(23)==2) {
					softPwmWrite(25,0);
				} else {
					softPwmWrite(25,50);
				};

			} while (!(ps3c_input(&ps3dat)));
		
			ps3c_exit(&ps3dat);		
		} else {
			// Automatic
			set_posLCD(0);
			put_LCDstring("Automatic");
//			set_posLCD(64);
//			sprintf(lcd_buff , " %5d",check_file("bar"));
//			put_LCDstring(lcd_buff);
//			sprintf(lcd_buff , " %8d",check_file("cntWheel"));
//			put_LCDstring(lcd_buff);

//			bmeter = 2; // imai
			
			while(1) {
				
				if(!digitalRead( 5)) {
					softPwmWrite(16, 0);softPwmWrite( 1, 0);
					softPwmWrite(28, 0);softPwmWrite(29, 0);
					bmeter = 0;
				};
				
				if(digitalRead( 3)) {
					digitalWrite(4,1);
					softPwmWrite(16, 0);softPwmWrite( 1, 0);
					softPwmWrite(28, 0);softPwmWrite(29, 0);
					break;
				};

				if (bmeter == 0) {
					set_posLCD(0);
					put_LCDstring("Automatic");
					set_posLCD(64);
					put_LCDstring("                    ");
					if(!digitalRead( 6)) {
						write_file("cntWheel" , 0 );
						write_file("bar" , 0 );
						system("mpg123 /home/pi/Music/victory_for_you.mp3 &");delay(3000);
						system("mpg123 /home/pi/Music/10.mp3 &");delay(1000);
						system("mpg123 /home/pi/Music/09.mp3 &");delay(1000);
						system("mpg123 /home/pi/Music/08.mp3 &");delay(1000);
						system("mpg123 /home/pi/Music/07.mp3 &");delay(1000);
						system("mpg123 /home/pi/Music/06.mp3 &");delay(1000);
						system("mpg123 /home/pi/Music/05.mp3 &");delay(1000);
						system("mpg123 /home/pi/Music/04.mp3 &");delay(1000);
						system("mpg123 /home/pi/Music/03.mp3 &");delay(1000);
						system("mpg123 /home/pi/Music/02.mp3 &");delay(1000);
						system("mpg123 /home/pi/Music/01.mp3 &");delay(1000);
						system("mpg123 /home/pi/Music/amuro.mp3 &");
			
						bmeter = 1;
					};
				};
				
				if(bmeter == 1) {
					softPwmWrite(16,80);softPwmWrite( 1, 0);
					softPwmWrite(28,80);softPwmWrite(29, 0);
					if(check_file("bar") > 24) {
						printf("%d\n",check_file("bar"));
						bmeter = 2;
					};
				};
				
				if(bmeter == 2) {
					softPwmWrite(16, 0);softPwmWrite( 1, 0);
					softPwmWrite(28, 0);softPwmWrite(29, 0);
					sleep(1);
				
					printf(" %4d , %4d , %4d , %4d \n",digitalRead(21),digitalRead(22),digitalRead(23),digitalRead(24));
//					if(1) {
//						softPwmWrite(16,8);softPwmWrite( 1, 0);//while(!digitalRead(21));
//					} else {softPwmWrite
//						softPwmWrite(28,8);softPwmWrite(29, 0);//while(!digitalRead(24));
//					};
//					sleep(1);


					while(digitalRead(21)+digitalRead(22)+digitalRead(23)+digitalRead(24)!=0) {
						softPwmWrite(16,8);softPwmWrite( 1, 0);
						softPwmWrite(28,8);softPwmWrite(29, 0);
					};
				
					softPwmWrite(16, 0);softPwmWrite( 1, 0);
					softPwmWrite(28, 0);softPwmWrite(29, 0);
					sleep(1);
					
					while(!(digitalRead(21)+digitalRead(24)!=0)) {
						softPwmWrite(16,8);softPwmWrite( 1, 0);
						softPwmWrite(28,8);softPwmWrite(29, 0);
					};

					softPwmWrite(16, 0);softPwmWrite( 1, 0);
					softPwmWrite(28, 0);softPwmWrite(29, 0);
					sleep(1);
					
					printf(" %4d , %4d , %4d , %4d \n",digitalRead(21),digitalRead(22),digitalRead(23),digitalRead(24));

					if(digitalRead(21)+digitalRead(24)==2) {
						softPwmWrite(16,8);softPwmWrite( 1, 0);
						softPwmWrite(28,8);softPwmWrite(29, 0);
					} else if(digitalRead(21)) {
						softPwmWrite(28,8);softPwmWrite(29, 0);
					} else if (digitalRead(24)){
						softPwmWrite(16,8);softPwmWrite( 1, 0);
					};
					sleep(1);
		
					softPwmWrite(16, 0);softPwmWrite( 1, 0);
					softPwmWrite(28, 0);softPwmWrite(29, 0);
					sleep(1);
							
					write_file("cntWheel" , 0 );
					write_file("bar" , 0 );
					
					bmeter = 3;// 0 
				};
				
				if(bmeter == 3) {
					softPwmWrite(16,80);softPwmWrite( 1, 0);
					softPwmWrite(28,80);softPwmWrite(29, 0);
					
					bmeter = 4;
				};
				
				if(bmeter == 4) {
					delay(2000);//7.6-2000
//					if(check_file("cntWheel") > 20) {
						printf("%d\n",check_file("cntWheel"));
						softPwmWrite(16, 0);softPwmWrite( 1, 0);
						softPwmWrite(28, 0);softPwmWrite(29, 0);
						sleep(1);
				
						digitalWrite(7,1);
						system("mpg123 /home/pi/Music/fire2.mp3");
						system("mpg123 /home/pi/Music/beamRifle.mp3 &");
						sleep(1);
						digitalWrite(7,0);
						sleep(1); // delete item
						
						write_file("cntWheel" , 0 );
						write_file("bar" , 0 );
						
						bmeter = 5;
//					};
				};
				
				if(bmeter == 5) {
					softPwmWrite(16,70);softPwmWrite( 1, 0);
					softPwmWrite(28,70);softPwmWrite(29, 0);

					if(check_file("bar") > 34) { //36
						printf("%d\n",check_file("bar"));
						softPwmWrite(16, 0);softPwmWrite( 1, 0);
						softPwmWrite(28, 0);softPwmWrite(29, 0);
						sleep(1);

						write_file("cntWheel" , 0 );
						write_file("bar" , 0 );

						bmeter = 6;
					};
				};
				
				if(bmeter == 6) {
					softPwmWrite(16,80);softPwmWrite( 1, 0);
					softPwmWrite(28,80);softPwmWrite(29, 0);

					delay(600);
//					if(check_file("cntWheel") > 5) { // 4
						printf("%d\n",check_file("cntWheel"));
						bmeter = 7;
//					};	
				};
				
				if(bmeter == 7) {
					write_file("cntWheel" , 0 );
					write_file("bar" , 0 );

					softPwmWrite(16, 0);softPwmWrite( 1, 0);
					softPwmWrite(28, 0);softPwmWrite(29, 0);
					sleep(1);

					set_posLCD(0);
					put_LCDstring("Terminate");
					set_posLCD(64);
					put_LCDstring("                    ");
					sleep(1);

					bmeter = 8;
				};
				
				if(bmeter == 8) {
					set_posLCD(0);
					put_LCDstring("Automatic");
					set_posLCD(64);
					put_LCDstring("Part 2        ");

					if(check_file("ball")) {	
						write_file("cntWheel" , 0 );
						write_file("bar" , 0 );
						system("mpg123 /home/pi/Music/otsukare.mp3");delay(900);
						system("mpg123 /home/pi/Music/10.mp3 &");delay(1000);
						system("mpg123 /home/pi/Music/09.mp3 &");delay(1000);
						system("mpg123 /home/pi/Music/08.mp3 &");delay(1000);
						system("mpg123 /home/pi/Music/07.mp3 &");delay(1000);
						system("mpg123 /home/pi/Music/06.mp3 &");delay(1000);
						system("mpg123 /home/pi/Music/05.mp3 &");delay(1000);
						system("mpg123 /home/pi/Music/04.mp3 &");delay(1000);
						system("mpg123 /home/pi/Music/03.mp3 &");delay(1000);
						system("mpg123 /home/pi/Music/02.mp3 &");delay(1000);
						system("mpg123 /home/pi/Music/01.mp3 &");delay(1000);
						system("mpg123 /home/pi/Music/amuro.mp3 &");
						bmeter = 1;
					};
				};
				
				if(bmeter == 9) {
					bmeter = 0;
				};
				
				



			};
		};
}

