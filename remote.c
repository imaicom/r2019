// cc -o remote remote.c -lwiringPi -lm

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/joystick.h>

#include <wiringPi.h>
#include <softPwm.h>

#include "controller.h"
#include <math.h>


struct ps3ctls {

	int fd;
	unsigned char nr_buttons;	// Max number of Buttons
	unsigned char nr_sticks;	// Max number of Sticks
	short *button;				// button[nr_buttons]
	short *stick;					// stick[nr_sticks]
};


#define NumberOfButton 17


int btn[NumberOfButton] = {};
int b_btn[NumberOfButton] = {};
int fds;

int ready_Go = 0;
int tennisBallCatch = 0;
int dropGate = 0;

int mode = 0;


int resetPCA9685(int fd) {
	wiringPiI2CWriteReg8(fd,0,0);
}


int setPCA9685Freq(int fd , float freq) {
	float prescaleval;
	int prescale , oldmode , newmode;
	freq = 0.9 * freq;
	prescaleval = 25000000.0;
	prescaleval /= 4096.0;
	prescaleval /= freq;
	prescaleval -= 1.0;
	prescale = prescaleval + 0.5;
	oldmode = wiringPiI2CReadReg8(fd,0x00);
	newmode = (oldmode & 0x7F)|0x10;
	wiringPiI2CWriteReg8(fd , 0x00 , newmode);
	wiringPiI2CWriteReg8(fd , 0xFE , prescale);
	wiringPiI2CWriteReg8(fd , 0x00 , oldmode);
	sleep(0.005);
	wiringPiI2CWriteReg8(fd , 0x00 , oldmode | 0xA1);
}


int setPCA9685Duty(int fd , int channel , int off) {
	int channelpos;
	int on;

	on   = 0;
	off += 276;
	channelpos = 0x6 + 4 * channel;
	wiringPiI2CWriteReg16(fd , channelpos   , on  & 0x0FFF);
	wiringPiI2CWriteReg16(fd , channelpos+2 , off & 0x0FFF);
}


int ps3c_test(struct ps3ctls *ps3dat) {

	unsigned char nr_btn = ps3dat->nr_buttons;
	unsigned char nr_stk = ps3dat->nr_sticks;
	int c1,c2,c3,c4;


	if(ps3dat->button[PAD_KEY_RIGHT]) btn[PAD_KEY_RIGHT]++;
	if(!ps3dat->button[PAD_KEY_RIGHT]) btn[PAD_KEY_RIGHT] = 0;
	if(b_btn[PAD_KEY_RIGHT] > btn[PAD_KEY_RIGHT]) {
		mode++; if(mode > 9) mode = 0; 
		if(mode == 0) {system("mpg123 /home/pi/Music/zero.mp3");};	
		if(mode == 1) {system("mpg123 /home/pi/Music/one.mp3");};
		if(mode == 2) {system("mpg123 /home/pi/Music/two.mp3");};
		if(mode == 3) {system("mpg123 /home/pi/Music/three.mp3");};
		if(mode == 4) {system("mpg123 /home/pi/Music/four.mp3");};	
		if(mode == 5) {system("mpg123 /home/pi/Music/five.mp3");};	
		if(mode == 6) {system("mpg123 /home/pi/Music/six.mp3");};	
		if(mode == 7) {system("mpg123 /home/pi/Music/seven.mp3");};	
		if(mode == 8) {system("mpg123 /home/pi/Music/eight.mp3");};	
		if(mode == 9) {system("mpg123 /home/pi/Music/nine.mp3");};	
	};
	b_btn[PAD_KEY_RIGHT] = btn[PAD_KEY_RIGHT];


	if(ps3dat->button[PAD_KEY_LEFT]) btn[PAD_KEY_LEFT]++;
	if(!ps3dat->button[PAD_KEY_LEFT]) btn[PAD_KEY_LEFT] = 0;
	if(b_btn[PAD_KEY_LEFT] > btn[PAD_KEY_LEFT]) {
		mode--; if(mode < 0) mode = 9; 
		if(mode == 0) {system("mpg123 /home/pi/Music/zero.mp3");};	
		if(mode == 1) {system("mpg123 /home/pi/Music/one.mp3");};
		if(mode == 2) {system("mpg123 /home/pi/Music/two.mp3");};
		if(mode == 3) {system("mpg123 /home/pi/Music/three.mp3");};
		if(mode == 4) {system("mpg123 /home/pi/Music/four.mp3");};	
		if(mode == 5) {system("mpg123 /home/pi/Music/five.mp3");};	
		if(mode == 6) {system("mpg123 /home/pi/Music/six.mp3");};	
		if(mode == 7) {system("mpg123 /home/pi/Music/seven.mp3");};	
		if(mode == 8) {system("mpg123 /home/pi/Music/eight.mp3");};	
		if(mode == 9) {system("mpg123 /home/pi/Music/nine.mp3");};	
	};
	b_btn[PAD_KEY_LEFT] = btn[PAD_KEY_LEFT];


	if(mode == 0) {setPCA9685Duty(fds , 9 , -120);		setPCA9685Duty(fds , 10 ,-90);	};	
	if(mode == 1) {setPCA9685Duty(fds , 9 , 0);		setPCA9685Duty(fds , 10 ,-90);	};
	if(mode == 2) {setPCA9685Duty(fds , 9 , +90);		setPCA9685Duty(fds , 10 ,-90);	};
	if(mode == 3) {setPCA9685Duty(fds , 9 , +90);		setPCA9685Duty(fds , 10 ,+60);	};
	if(mode == 4) {setPCA9685Duty(fds , 9 , +30);		setPCA9685Duty(fds , 10 ,+60);	};	
	if(mode == 5) {setPCA9685Duty(fds , 9 , -120);		setPCA9685Duty(fds , 10 ,+60);	};	
	if(mode == 6) {setPCA9685Duty(fds , 9 , 0);		setPCA9685Duty(fds , 10 ,+60);	};	
	if(mode == 7) {setPCA9685Duty(fds , 9 , +90);		setPCA9685Duty(fds , 10 ,+60);	};	
	if(mode == 8) {setPCA9685Duty(fds , 9 , +90);		setPCA9685Duty(fds , 10 ,-90);	};	
	if(mode == 9) {setPCA9685Duty(fds , 9 , +60);		setPCA9685Duty(fds , 10 ,-90);	};	


	if(ps3dat->button[PAD_KEY_TRIANGLE]) btn[PAD_KEY_TRIANGLE]++;
	if(!ps3dat->button[PAD_KEY_TRIANGLE]) btn[PAD_KEY_TRIANGLE] = 0;
	if(b_btn[PAD_KEY_TRIANGLE] > btn[PAD_KEY_TRIANGLE]) {
		dropGate = 110 - dropGate;
		if(dropGate == 0)		system("mpg123 /home/pi/Music/lift.mp3");
		if(dropGate == 110)	system("mpg123 /home/pi/Music/drop.mp3");
	};
	b_btn[PAD_KEY_TRIANGLE] = btn[PAD_KEY_TRIANGLE];

	setPCA9685Duty(fds , 5 , dropGate);
	setPCA9685Duty(fds , 6 , -dropGate);


// Drive train

    c1 = -ps3dat->stick[PAD_RIGHT_Y] / 4;
    c2 = -ps3dat->stick[PAD_RIGHT_X];
    c3 = ps3dat->stick[PAD_LEFT_X];
    c4 = ps3dat->stick[PAD_LEFT_Y];

	if(c3 > +70) {
		softPwmWrite( 5, 15); softPwmWrite( 6, 0); //Right rotation
		softPwmWrite(26, 15); softPwmWrite(27, 0);
		softPwmWrite(28,  0); softPwmWrite(29,20);
		softPwmWrite( 1,  0); softPwmWrite( 4,20);	
	}else if(c3 < -70) {
		softPwmWrite( 5, 0); softPwmWrite( 6, 15); //Left rotation
		softPwmWrite(26, 0); softPwmWrite(27, 15);
		softPwmWrite(28,20); softPwmWrite(29,  0);
		softPwmWrite( 1,20); softPwmWrite( 4,  0);
	}else { // other than c3

		if(c1 < -10) {
			softPwmWrite( 5, abs(c1)); softPwmWrite( 6, 0);
			softPwmWrite(27, abs(c1)); softPwmWrite(26, 0);
		} else if(c1 > +10) {
			softPwmWrite( 5, 0); softPwmWrite( 6, abs(c1));
			softPwmWrite(27, 0); softPwmWrite(26, abs(c1));
		} else {
			softPwmWrite( 5, 0); softPwmWrite( 6, 0);
			softPwmWrite(26, 0); softPwmWrite(27, 0);
		}; // end of if(c1 < -10) 
	
		if(c2 < -10) {
			softPwmWrite(28, abs(c2)); softPwmWrite(29, 0);
			softPwmWrite( 1, 0); softPwmWrite( 4, abs(c2));	
		} else if(c2 > +10) {
			softPwmWrite(28, 0); softPwmWrite(29, abs(c2));
			softPwmWrite( 1, abs(c2)); softPwmWrite( 4, 0);
		} else {
			softPwmWrite(28, 0);	softPwmWrite(29, 0);
			softPwmWrite( 1, 0);	softPwmWrite( 4, 0);	
		}; // end of if(c2 < -10) 
	
	};// end of other than c3

// end of Drive train
		
		
	if((ps3dat->button[PAD_KEY_PS])&&(!ready_Go)) {
		ready_Go = 1;
		system("mpg123 /home/pi/Music/ready_Go.mp3");
	};
	
	
	if(ps3dat->button[PAD_KEY_L1]) {
		setPCA9685Duty(fds , 8 , +50);
	} else if(ps3dat->button[PAD_KEY_L2]) {
		setPCA9685Duty(fds , 8 , -50);
	} else {
		setPCA9685Duty(fds , 8 ,   0);
	};


	if(ps3dat->button[PAD_KEY_R1]) {
		setPCA9685Duty(fds , 7 , -50);
	} else if(ps3dat->button[PAD_KEY_R2]) {
		setPCA9685Duty(fds , 7 , +50);
	} else {
		setPCA9685Duty(fds , 7 ,   0);
	};


	if(ps3dat->button[PAD_KEY_UP]) {
		softPwmWrite(15, 100); softPwmWrite(11, 100);
	} else if(ps3dat->button[PAD_KEY_DOWN]) {
		softPwmWrite(16, 100); softPwmWrite(10, 100);
	} else {
		softPwmWrite(15, 0); softPwmWrite(16, 0); softPwmWrite(10, 0); softPwmWrite(11, 0);
	};


	if(ps3dat->button[PAD_KEY_CIRCLE]) {
		setPCA9685Duty(fds , 1 , +50);	// servo death
		softPwmWrite(30, 100);
	} else if(ps3dat->button[PAD_KEY_SQUARE]) {
		setPCA9685Duty(fds , 1 , -50);	// servo death
		softPwmWrite(31, 100);
	} else {
		setPCA9685Duty(fds , 1 ,   0);	// servo death
		softPwmWrite(30, 0); softPwmWrite(31, 0);
	};
	
	if(c4 > +70) {
		setPCA9685Duty(fds , 0 , -50);	// servo death
		softPwmWrite(22, 100);
	}else if((c4< -70)||(ps3dat->button[PAD_KEY_PS])) {
		setPCA9685Duty(fds , 0 , +50);	// servo death
		softPwmWrite(21, 40);
	}else {
		setPCA9685Duty(fds , 0 ,   0);	// servo death
		softPwmWrite(21, 0); softPwmWrite(22, 0);
	};


	if(ps3dat->button[PAD_KEY_CROSS]) btn[PAD_KEY_CROSS]++;
	if(!ps3dat->button[PAD_KEY_CROSS]) btn[PAD_KEY_CROSS] = 0;
	if(b_btn[PAD_KEY_CROSS] > btn[PAD_KEY_CROSS]) {
		tennisBallCatch = 100 - tennisBallCatch;
		if(tennisBallCatch == 0) 	system("mpg123 /home/pi/Music/lock.mp3 &");
		if(tennisBallCatch == 100) 	system("mpg123 /home/pi/Music/release.mp3 &");	
	};
	b_btn[PAD_KEY_CROSS] = btn[PAD_KEY_CROSS];
	
	setPCA9685Duty(fds , 2 , tennisBallCatch);
	setPCA9685Duty(fds , 3 , tennisBallCatch);
	setPCA9685Duty(fds , 4 , tennisBallCatch);


	if(ps3dat->button[PAD_KEY_START]) {
		system("mpg123 /home/pi/Music/turnOff.mp3");
		softPwmWrite( 5,0); // motor-1 10ms
		softPwmWrite( 6,0); // motor-1 10ms
		softPwmWrite(26,0); // motor-2 10ms
		softPwmWrite(27,0); // motor-2 10ms
		softPwmWrite( 1,0); // motor-3 10ms
		softPwmWrite( 4,0); // motor-3 10ms
		softPwmWrite(28,0); // motor-4 10ms
		softPwmWrite(29,0); // motor-4 10ms
		softPwmWrite(25,0); // motor-5 10ms // NC
		softPwmWrite(24,0); // motor-5 10ms // NC
		softPwmWrite(3,0); // beep

		softPwmWrite(15,0); // motor-6 10ms
		softPwmWrite(16,0); // motor-6 10ms
		softPwmWrite(10,0); // motor-7 10ms
		softPwmWrite(11,0); // motor-7 10ms
		softPwmWrite(30,0); // motor-8 10ms
		softPwmWrite(31,0); // motor-8 10ms
		softPwmWrite(21,0); // motor-9 10ms
		softPwmWrite(22,0); // motor-9 10ms
		system("mpg123 /home/pi/Music/byeBye.mp3");

		return -1; // end of program
	};

	return 0;

}	//	int ps3c_test(struct ps3ctls *ps3dat)


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
				ps3dat->stick[ev.number] = ev.value / 200; // 327 range -32767 ~ +32768 -> -100 ~ +100
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
	int i;

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

	wiringPiSetup();
	softPwmCreate( 5,0,20); // motor-1 10ms
	softPwmCreate( 6,0,20); // motor-1 10ms
	softPwmCreate(26,0,20); // motor-2 10ms
	softPwmCreate(27,0,20); // motor-2 10ms
	softPwmCreate( 1,0,20); // motor-3 10ms
	softPwmCreate( 4,0,20); // motor-3 10ms
	softPwmCreate(28,0,20); // motor-4 10ms
	softPwmCreate(29,0,20); // motor-4 10ms
	softPwmCreate(25,0,20); // motor-5 10ms
	softPwmCreate(24,0,20); // motor-5 10ms
	softPwmCreate( 3,0,20); // beep

	softPwmCreate(15,0,20); // motor-6 10ms
	softPwmCreate(16,0,20); // motor-6 10ms
	softPwmCreate(10,0,20); // motor-7 10ms
	softPwmCreate(11,0,20); // motor-7 10ms
	softPwmCreate(30,0,20); // motor-8 10ms
	softPwmCreate(31,0,20); // motor-8 10ms
	softPwmCreate(21,0,20); // motor-9 10ms
	softPwmCreate(22,0,20); // motor-9 10ms

	fds = wiringPiI2CSetup(0x40);	// PCA9685
	resetPCA9685(fds);
	setPCA9685Freq(fds,50);
	system("mpg123 /home/pi/Music/Main_system_startup.mp3");
	delay(200);
	system("mpg123 /home/pi/Music/Press_the_PS_button.mp3");


	if(!(ps3c_init(&ps3dat, df))) {

		setPCA9685Duty(fds , 0 ,   0);
		setPCA9685Duty(fds , 1 ,   0);
		setPCA9685Duty(fds , 2 ,   0);
		setPCA9685Duty(fds , 3 ,   0);
		setPCA9685Duty(fds , 4 ,   0);
		setPCA9685Duty(fds , 5 ,   0);
		setPCA9685Duty(fds , 6 ,   0);
		setPCA9685Duty(fds , 7 ,   0);
		setPCA9685Duty(fds , 8 ,   0);
		setPCA9685Duty(fds , 9 ,   0);
		setPCA9685Duty(fds , 10,   0);
		setPCA9685Duty(fds , 11,   0);
		setPCA9685Duty(fds , 12,   0);
		setPCA9685Duty(fds , 13,   0);


		do {
			if (ps3c_test(&ps3dat) < 0) break;
		} while (!(ps3c_input(&ps3dat)));

		ps3c_exit(&ps3dat);
	}
}

