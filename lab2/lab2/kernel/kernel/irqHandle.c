#include "x86.h"
#include "device.h"

extern int displayRow;
extern int displayCol;

extern uint32_t keyBuffer[MAX_KEYBUFFER_SIZE];
extern int bufferHead;
extern int bufferTail;


int is_enter = 0;

void GProtectFaultHandle(struct TrapFrame *tf);
void moveStr(char *buf,int len, char *dst);
void KeyboardHandle(struct TrapFrame *tf);
void timerHandle(struct TrapFrame *tf);
void syscallHandle(struct TrapFrame *tf);
void syscallWrite(struct TrapFrame *tf);
void syscallPrint(struct TrapFrame *tf);
void syscallRead(struct TrapFrame *tf);
void syscallGetChar(struct TrapFrame *tf);
void syscallGetStr(struct TrapFrame *tf);

void irqHandle(struct TrapFrame *tf) { // pointer tf = esp
	/*
	 * 中断处理程序
	 */
	/* Reassign segment register */
	asm volatile("movw %%ax, %%ds"::"a"(KSEL(SEG_KDATA)));
	//asm volatile("movw %%ax, %%es"::"a"(KSEL(SEG_KDATA)));
	//asm volatile("movw %%ax, %%fs"::"a"(KSEL(SEG_KDATA)));
	//asm volatile("movw %%ax, %%gs"::"a"(KSEL(SEG_KDATA)));
	//putint(tf->irq);
	switch(tf->irq) {
		// TODO: 填好中断处理程序的调用
		case -1:
			break;
		case 0xd:
			GProtectFaultHandle(tf);
			break;
		case 0x20:
			timerHandle(tf);
			break;
		case 0x21:
			KeyboardHandle(tf);
			break;
		case 0x80:
			syscallHandle(tf);
			break;
		default:assert(0);
	}
}

void GProtectFaultHandle(struct TrapFrame *tf){
	assert(0);
	return;
}
void timerHandle(struct TrapFrame *tf) {
	return;
}
void KeyboardHandle(struct TrapFrame *tf){
	uint32_t code = getKeyCode();
	if(code == 0xe){ // 退格符
		// TODO: 要求只能退格用户键盘输入的字符串，且最多退到当行行首
		if(displayCol > 0 && bufferTail > 0){
			displayCol--;
			bufferTail--;
			uint16_t data = 0 | (0x0c << 8);
			int pos = (80 * displayRow + displayCol) * 2;
			asm volatile("movw %0, (%1)" ::"r"(data), "r"(pos + 0xb8000));
		}
		//scrollScreen();
	}else if(code == 0x1c){ // 回车符
		// TODO: 处理回车情况
		//displayRow++;
		displayCol = 0;
		if (displayRow == 25)
		{
			displayRow = 24;
			displayCol = 0;
			scrollScreen();
		}
		is_enter = 1;
	}else if(code < 0x81){ // 正常字符
		// TODO: 注意输入的大小写的实现、不可打印字符的处理
		if(code == 0x2a || code == 0x2a + 0x80 || code == 0x36 || code == 0x36 + 0x80 ||code == 0x3a || code == 0x3a + 0x80)
			return;
		char asc = getChar(code);
		uint16_t data = asc| (0x0c << 8);
		int pos = (80 * displayRow + displayCol) * 2;
		asm volatile("movw %0, (%1)" ::"r"(data), "r"(pos + 0xb8000));
		displayCol++;
		if (displayCol == 80)
		{
			displayRow++;
			displayCol = 0;
			if (displayRow == 25)
			{
				displayRow = 24;
				displayCol = 0;
				scrollScreen();
			}
		}
		keyBuffer[bufferTail] = asc;
		bufferTail++;
	}
	updateCursor(displayRow, displayCol);
}

void syscallHandle(struct TrapFrame *tf) {
	switch(tf->eax) { // syscall number
		case 0:
			syscallWrite(tf);
			break; // for SYS_WRITE
		case 1:
			syscallRead(tf);
			break; // for SYS_READ
		default:break;
	}
}

void syscallWrite(struct TrapFrame *tf) {
	switch(tf->ecx) { // file descriptor
		case 0:
			syscallPrint(tf);
			break; // for STD_OUT
		default:break;
	}
}

void syscallPrint(struct TrapFrame *tf) {
	int sel =  USEL(SEG_UDATA); //TODO: segment selector for user data, need further modification
	char *str = (char*)tf->edx;
	///putInt((uint32_t)str);
	int size = tf->ebx;
	int i = 0;
	int pos = 0;
	char character = 0;
	uint16_t data = 0;
	asm volatile("movw %0, %%es"::"m"(sel));
	for (i = 0; i < size; i++) {
		asm volatile("movb %%es:(%1), %0":"=r"(character):"r"(str+i));
		// TODO: 完成光标的维护和打印到显存
		if (character == '\n')
		{
			displayRow++;
			displayCol = 0;
			if (displayRow == 25)
			{
				displayRow = 24;
				displayCol = 0;
				scrollScreen();
			}
		}
		else
		{
			data = character | (0x0c << 8);
			pos = (80 * displayRow + displayCol) * 2;
			asm volatile("movw %0, (%1)" ::"r"(data), "r"(pos + 0xb8000));
			displayCol++;
			if (displayCol == 80)
			{
				displayRow++;
				displayCol = 0;
				if (displayRow == 25)
				{
					displayRow = 24;
					displayCol = 0;
					scrollScreen();
				}
			}
		}
	}
	
	updateCursor(displayRow, displayCol);
	return;
}

void syscallRead(struct TrapFrame *tf){
	switch(tf->ecx){ //file descriptor
		case 0:
			syscallGetChar(tf);
			break; // for STD_IN
		case 1:
			syscallGetStr(tf);
			break; // for STD_STR
		default:break;
	}
}

void syscallGetChar(struct TrapFrame *tf){
	// TODO: 自由
	//char *str = (char*)tf->edx;
	//int sel =  USEL(SEG_UDATA); 
	//asm volatile("movw %0, %%es"::"m"(sel));
	uint32_t code = getKeyCode();
	while(!code){
		code = getKeyCode();
	}
	char asc = getChar(code);
	uint16_t data = asc| (0x0c << 8);
	int pos = (80 * displayRow + displayCol) * 2;
	asm volatile("movw %0, (%1)" ::"r"(data), "r"(pos + 0xb8000));
	displayCol++;
	if (displayCol == 80)
	{
		displayRow++;
		displayCol = 0;
		if (displayRow == 25)
		{
			displayRow = 24;
			displayCol = 0;
			scrollScreen();
		}
	}
	updateCursor(displayRow, displayCol);
	while(1){
		//putString("here\n");
		//putInt(code);
		code = getKeyCode();
		char character = getChar(code);
		//putChar(character);
		if (character == '\n')
		{
			displayRow++;
			displayCol = 0;
			if (displayRow == 25)
			{
				displayRow = 24;
				displayCol = 0;
				scrollScreen();
			}
			updateCursor(displayRow, displayCol);
			break;
		}

	}
	//asm volatile("movb %0, %%es:(%1)"::"r"(asc),"r"(str));
	//asm volatile("movb %0, %%eax":"=m"(asc));

	tf->eax = asc;
}

void syscallGetStr(struct TrapFrame *tf){ 
	char *str = (char*)tf->edx;
	//str += 0x200000;
	//putInt((uint32_t)str);
	//int size = tf->ebx;        
	char buffer[256];
	int i = 0;                          
	uint32_t code = getKeyCode();
	while(code == 0x1c || code == 0x1c + 0x80){
		code = getKeyCode();
	}
	uint32_t temp;
	while(1){
		//putString("here\n");
		//putInt(code);
		code = getKeyCode();
		char character = getChar(code);
		//putChar(character);
		if (character == '\n')
		{
			displayRow++;
			displayCol = 0;
			if (displayRow == 25)
			{
				displayRow = 24;
				displayCol = 0;
				scrollScreen();
			}
			updateCursor(displayRow, displayCol);
			break;
		}
		temp = getKeyCode();
		while(temp == code){
			temp = getKeyCode();
		}
		if(temp == 0xe){ // 退格符
			if(displayCol > 0){
				displayCol--;
				uint16_t data = 0 | (0x0c << 8);
				int pos = (80 * displayRow + displayCol) * 2;
				asm volatile("movw %0, (%1)" ::"r"(data), "r"(pos + 0xb8000));
			}
			if(i >0)i--;
		}
		else if(temp != 0x0 && temp != 0x2a && temp != 0x2a + 0x80 && temp != 0x36 && temp != 0x36 + 0x80 
				&&temp != 0x3a &&temp != 0x3a + 0x80 && temp != 0x1c){
			char asc = getChar(temp);
			uint16_t data = asc| (0x0c << 8);
			int pos = (80 * displayRow + displayCol) * 2;
			asm volatile("movw %0, (%1)" ::"r"(data), "r"(pos + 0xb8000));
			if(temp - code != 128){
				displayCol += 1;
				buffer[i] = asc;
				i++;
			}
			if (displayCol == 80)
			{
				displayRow++;
				displayCol = 0;
				if (displayRow == 25)
				{
					displayRow = 24;
					displayCol = 0;
					scrollScreen();
				}
			}
			updateCursor(displayRow, displayCol);
		}
	}
	//putInt((uint32_t)str);
	///putInt(i);
	for(int j = 0;j < i;j++)	
		putChar(buffer[j]);
	moveStr(buffer,i,(char*)tf->edx);
}


void moveStr(char *buf,int len, char *dst){
	int i = 0;
	for(;i < len;i++)
		dst[i] = buf[i];
	dst[i] = 0;
}