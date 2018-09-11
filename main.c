#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
//declare all global variables (registers, pointers etc)
char * IP; // instrnction pointer
int initial;
int a;
//flags
int x[3]={0,0,0};
int * flags= &x[0];
// registers
int reg[8]={0,1,2,3,4,5,6,7};
const char delim[] = " .\t\n\v\f\r";
// bitfield to store data
typedef struct
    {
        unsigned char high:4;
        unsigned char low:4;
    }bitfield_val;


void text_dir(char * ptr,char * start_addr,char * data)
{
     bitfield_val * toStore = (bitfield_val*) ptr+strtol(start_addr,NULL,16); // point the bitfield at the region where you want to put the stuff in
     int i=0;
     initial=strtol(start_addr,NULL,16);

     // loop through the string, converting the ascii string into a numerical value to store. (subtract the offsets)
    for(i=0;i<strlen(data)-1;i=i+2)
        {
            if(data[i]<58)
                {
                    toStore -> low = data[i]-48;
                }
            if(data[i]>58)
                {
                    toStore -> low = data[i]-87;
                }
            if(data[i+1]<58)
                {
                    toStore -> high = data[i+1]-48;
                }
            if(data[i+1]>58)
                {
                    toStore -> high = data[i+1]-87;
                }

            toStore=toStore+1; // iterate the bitfield to move to the next region
        }
}

void byte_dir(char * ptr,char * address,char * data)
{
    int addr=strtol(address,NULL,16);
    int value=strtol(data,NULL,16);
    ptr[addr]=(char)value;
}
void long_dir(char * ptr, char * address, char * data) // still needs to be tested along with the string directive
{
    int * toStore = (int *) ptr + strtol(address,NULL,16); // point this to the right region in memory
    int data_val=strtol(data,NULL,10); // convert the string value to a 4 btye value
    *toStore=data_val; // assign this region a long value
}
void string_dir(char * ptr,char* address, char * data)
{
    // ask if we need to do error checking for this project (most likely, we will and then i need to edit these 4 functions
    int len = strlen(data); // get the length of the string in question
    char newstring[len-1];
    newstring[len-2]='\0'; // make sure that it is a null terminated string
    strncpy(newstring,data+1,len-2); // copy just the string part into the program
    len=strlen(newstring);
    int i=0;
    char * toStore = ptr+strtol(address,NULL,16);
    for(i=0;i<len+1;i++)
        {
            *toStore=newstring[i];
            toStore=toStore+1;
        }
}
void nop()
{
    initial=initial+1;
}
void halt(char * ptr)
{
    free((void*)ptr);
    printf("Halt");
    return;
}
void rrmovl(char * ptr, int * reg)
{
    bitfield_val * registers= (bitfield_val*)&ptr[initial+1]; // point this at the relevant stuff
    int low=(int)registers->low;
    int high=(int)registers->high;
    if((low<8 && low>-1) &&(high<8 && high>-1))
        {
            reg[high]=reg[low];
            initial=initial+2;
        }
    else
        {
            fprintf(stderr,"INS : register values must be between 0-7");
            exit(10);
        }
}
void irmovl(char * ptr, int * reg)
{
    bitfield_val * values = (bitfield_val*)&ptr[initial+1]; // pointed at register in question
    int imm=values->low;
    int reg_num=values->high;
    if(imm==15 &&(reg_num>-1 && reg_num<8))
        {
            int * number=(int*)(&ptr[initial+2]);
            reg[reg_num]=*number;
            initial=initial+6; // iterate the pointer value

        }
    else
        {
            fprintf(stderr,"INS : immediate not 15 or register invalid \n");
            exit(5);
        }
}
void rmmovl(char * ptr,int * reg,char * start, int s) // s will be the size
{
    bitfield_val * registers = (bitfield_val*)&ptr[initial+1]; // point it at the right spot in memory
    int * displacement = (int*)(&ptr[initial+2]);
    int addr=reg[registers->high] + *displacement;
    if(registers->low<0 || registers->low>7)
        {
            fprintf(stderr,"INS: register values must be between 0-7");
            exit(10);
        }
    if(addr>-1 || addr<s)
        {
            int * realaddr=(int*)(&ptr[addr]);
            *realaddr=reg[registers->low];
            (initial)=(initial)+6;
        }
    else
        {
            fprintf(stderr,"ADR: Addressing error");
            exit(8);
        }
}
void mrmovl(char * ptr, int * reg,char *start, int s)
{
    bitfield_val * registers = (bitfield_val*)&ptr[initial+1];
    int * displacement = (int *)(&ptr[initial+2]);
    int addr=reg[registers->high]+*displacement;
    if(registers->low<0 || registers->low>7)
        {
            fprintf(stderr,"INS: register values must be between 0-7");
            exit(10);
        }
    if(addr>-1 && addr<s)
        {
            int * realaddr=(int *)(&ptr[addr]);
            reg[registers->low]=*realaddr;
            (initial)=(initial)+6;
        }
    else
        {
            fprintf(stderr,"ADR: addressing error");
            exit(30);
        }
}
void op1(char * ptr, int * reg, int * flags)
{
    // flags will go ZF,SF,OF (0,1,2)
    bitfield_val * opcode = (bitfield_val*) (&ptr[initial]);
    bitfield_val * registers=(bitfield_val *) (&ptr[initial+1]);
    int v1=0;
    int v2=0;
    int result=0;
    if(registers->high<0 || registers->high>7 || registers->low<0 || registers->low>7)
        {
            fprintf(stderr,"INS: register values must be between 0-7");
            exit(10);
        }
    switch(opcode->high)
    {
    case 0: // add
        v1=reg[registers->high];
        v2=reg[registers->low];
        int sum=v1+v2;
        if(sum==0)
            {
                flags[0]=1;
            }
        else
            {
                flags[0]=0;
            }
        if(sum<0)
            {
                flags[1]=1;
            }
        else
            {
                flags[1]=0;
            }
        if((v1<0 && v2<0 && sum >0)||(v1>0 && v2>0 && sum<0))
            {
                flags[2]=1;
            }
        else
            {
                flags[2]=0;
            }
        reg[registers->high]=sum;
        break;
    case 1: //subtract
         v1=reg[registers->high];
         v2=reg[registers->low];
        int diff=v1-v2;
        if(diff==0)
            {
               flags[0]=1;
            }
        else
            {
                flags[0]=0;
            }
        if(diff<0)
            {
                flags[1]=1;
            }
        else
            {
                flags[1]=0;
            }
        if((v1<0&&v2>0&&diff>0)||(v1>0&&v2<0&&diff<0))
            {
                flags[2]=1;
            }
        else
            {
                flags[2]=0;
            }
        reg[registers->high]=diff;
        break;
    case 2: // and
         v1=reg[registers->high];
         v2=reg[registers->low];
        result=v1 & v2;
        if(result==0)
            {
                flags[0]=1;
            }
        else
            {
                flags[0]=0;
            }
        if(result<0)
            {
                flags[1]=1;
            }
        else
            {
                flags[1]=0;
            }
        reg[registers->high]=result;
        break;
    case 3: // xor
         v1=reg[registers->high];
         v2=reg[registers->low];
         result=v1^v2;
        if(result==0)
            {
                flags[0]=1;
            }
        else
            {
                flags[0]=0;
            }
        if(result<0)
            {
                flags[1]=1;
            }
        else
            {
                flags[1]=0;
            }
        reg[registers->high]=result;
    case 4: //multiply
         v1=reg[registers->high];
         v2=reg[registers->low];
        int product=v1*v2;
        if(product==0)
            {
                flags[0]=1;
            }
        else
            {
                flags[0]=0;
            }
        if(product<0)
            {
                flags[1]=1;
            }
        else
            {
                flags[1]=0;
            }
        if((v1>0&&v2>0&&product<0)||(v1<0&&v2<0&&product<0)||(v1<0&&v2>0&&product>0)||(v1>0&&v2<0&&product>0))
            {
                flags[2]=1;
            }
        else
            {
                flags[2]=0;
            }
        reg[registers->high]=product;
        break;
    case 5:
         v1=reg[registers->high];
         v2=reg[registers->low];
        if(v1-v2==0)
            {
                flags[0]=1;
            }
        else
            {
                flags[0]=0;
            }
        if(v1-v2<0)
            {
                flags[1]=1;
            }
        else
            {
                flags[1]=0;
            }
        break;
    default:
        fprintf(stderr,"this has invalid operations on registers");
        exit(25);

}
    (initial)=(initial)+2;
}
void jxx(char * ptr,int * flags, char * start)
{
    //look at the flags and get the values from there
    bitfield_val * opcode = (bitfield_val *)(&ptr[initial]);
    int * dest = (int*)(&ptr[initial+1]);
    int ZF=flags[0];
    int SF=flags[1];
    int OF=flags[2];
    // either jump to the address if the conditions are net, or just iterate the pointer for the next instruction
    switch(opcode->high)
    {
        case 0: //jmp
            (initial)=(*dest);
            break;
        case 1: //jle
             if(((SF^OF)|ZF))
                {
                    (initial)=(*dest);
                }
            else
                {
                    (initial)=(initial)+5;
                }
                break;
        case 2: //jl
             if((SF^OF))
                {
                    (initial)=(*dest);
                }
            else
                {
                    (initial)=(initial)+5;
                }
                break;
        case 3: //je
             if(ZF)
                {
                    (initial)=(*dest);
                }
            else
                {
                    (initial)=(initial)+5;
                }
                break;
        case 4: //jne
             if(ZF==0)
                {
                    (initial)=(*dest);
                }
            else
                {
                    (initial)=(initial)+5;
                }
                break;
        case 5: //jge
             if(~(SF^OF))
                {
                    (initial)=(*dest);
                }
            else
                {
                    (initial)=(initial)+5;
                }
                break;
        case 6: //jg
             if(~(SF^OF)&~ZF)
                {
                    (initial)=(*dest);
                }
            else
                {
                    (initial)=(initial)+5;
                }
                break;
        default:
            fprintf(stderr,"INS: opcode is invalid in function jxx");
            exit(20);
    }
}
void call(char * ptr,int * reg,char * start,int s) // problem is how call is working!!!
{
    int * dest = (int*)(&ptr[initial+1]); // get the number
    // stack pointer is register 4
    (initial)=(initial)+5; // this is the next instruction
    reg[4]=reg[4]-4; // decrement the address in here
    if(reg[4]<0 || reg[4]>s-1)
        {
            fprintf(stderr,"ADR: address in register is outside the allocated block");
            exit(8);
        }
    *((int *)&ptr[reg[4]])=initial;
    // error check here later
    if(*dest>(s-1) || *dest <0)
        {
            fprintf(stderr, "ADR : call is out of bounds");
            exit(15);
        }
    (initial)=*dest;
}
void ret(char * ptr, int * reg, int s)
{
    if(reg[4]<0||reg[4]>s-1)
        {
            fprintf(stderr,"ADR: address in register is outside the allocated block");
            exit(8);
        }
    memcpy(&initial,&ptr[reg[4]],4);
    reg[4]=reg[4]+4;
}
void pushl(char * ptr, int * reg, int s)
{
    bitfield_val * field = (bitfield_val *)(&ptr[initial+1]);
    if((field->low>-1 && field->low<8)&&(field->high==15))
        {
            reg[4]=reg[4]-4;
            if(reg[4]<0 || reg[4]>s-1)
                {
                    fprintf(stderr,"ADR: address in register is outside the allocated block");
                    exit(8);
                }
            *(int*)(&ptr[reg[4]])=reg[field->low]; // changed to low
        }
    else
        {
            fprintf(stderr,"INS : invalid instruction ");
            exit(200);
        }
    (initial)=(initial)+2;
}
void popl(char * ptr, int * reg, int s)
{
    bitfield_val * field = (bitfield_val *)(&ptr[initial+1]);
    if((field->low>-1 && field->low<8)&&(field->high==15))
        {
            if(reg[4]<0 ||reg[4]>s-1)
                {
                    fprintf(stderr,"ADR: address in register is outside the allocated block");
                    exit(8);
                }
            reg[field->low]=*(int *)(&ptr[reg[4]]);
            reg[4]=reg[4]+4;
        }
    else
        {
            fprintf(stderr,"INS: invalid instruction");
            exit(200);
        }
    (initial)=(initial)+2;
}
void read(char * ptr,int * reg,int *flags, char * start)
{
    bitfield_val * opcode = (bitfield_val*)(&ptr[initial]);
    bitfield_val * registers = (bitfield_val*)(&ptr[initial+1]);
    int * displacement = (int*)(&ptr[initial+2]);
    if(opcode->high==0) // reading in just the byte
        {
            char input;
            printf("Enter a char: \n");
            scanf("%c",&input);
            if(input==EOF)
                {
                    flags[0]=1;
                }
            else
                {
                    flags[0]=0;
                    if(registers->low>-1 && registers->low<8 && registers->high==15)
                    {
                        char * memaddr= (&ptr[reg[registers->low]+*displacement]);
                        *memaddr=input;
                        (initial)=(initial)+6;
                    }
                     else
                        {
                            fprintf(stderr,"INS: This is invalid register to go to");
                            exit(10);
                        }
                }
        }
    else if(opcode->high==1)
        {
            int iinput;
            scanf("%d",&iinput);
            if(iinput<0)
                {
                    flags[0]=1;
                    printf("This is the end of the program: HALT");
                    exit(0);
                }
            else
                {
                    flags[0]=0;
                    if(registers->low>-1 && registers->low<8 && registers->high==15)
                        {
                            int * memaddr = (int*)(&ptr[reg[registers->low]+*displacement]);
                            *memaddr=iinput;
                            (initial)=(initial)+6;
                        }
                    else
                        {
                            fprintf(stderr,"INS: This opcode is invalid, one register should be f");
                            exit(10);
                        }
                }
        }
    else
        {
            fprintf(stderr,"INS: This is an invalid opcode for accessing registers");
            exit(10);
        }

}
void write(char * ptr, int * reg, char * start)
{
    bitfield_val * opcode = (bitfield_val*)(&ptr[initial]);
    bitfield_val * registers=(bitfield_val*)(&ptr[initial+1]);
    int * displacement = (int*)(&ptr[initial+2]);
    if(opcode->high==0) // write a byte
        {
            if(registers->high==15 && (registers->low>-1 && registers->low<8))
                {
                    (initial)=(initial)+6;
                    char * output=(char *)(&ptr[reg[registers->low]+*displacement]);
                    printf("%c",*output);

                }
            else
                {
                   fprintf(stderr,"INS: One reg should be 0-7, one should be 15");
                   exit(4);
                }
        }
    else if(opcode->high==1) // write a long
        {
             if(registers->high==15 && (registers->low>-1 && registers->low<8))
                {
                    (initial)=(initial)+6;
                    int * ioutput=(int *)(&ptr[reg[registers->low]+*displacement]);
                    printf("%i",*ioutput);
                }
            else
            {
                fprintf(stderr,"INS: one reg should be 0-7, one should be 15");
                exit(4);
            }

        }
    else
        {
            fprintf(stderr,"INS: This is an invalid opcode");
            exit(10);
        }
    return;

}
void movsbl(char * ptr,int * reg,char * start,int s)
{
    bitfield_val * registers = (bitfield_val*)(&ptr[initial+1]);
    int * displacement = (int*)(&ptr[initial+2]);
    char * byteval =(char *)(&ptr[reg[registers->high]+*displacement]);
    int value = (int)(int8_t)(*byteval);
    if(value>s-1 || value <0)
        {
            fprintf(stderr,"ADR: address is outside the block allocated");
            exit(8);
        }
    reg[registers->low]=value;
    (initial)=(initial)+6;
}
int main(int argc, char * argv[])
{
    if(argc!=2)
        {
            printf("Input format is incorrect. Please enter only one program name.");
            exit(3);
        }
    if (strcmp(argv[1], "-h") == 0)
        {
		printf("Usage: filename (example: prog1.y86)");
		exit(0);
        }
    FILE * f1,*fp;
    f1=fopen(argv[1],"r");
    if (f1==NULL)
        {
           printf("error: file is empty");
            exit(10);
        }
    int count=0;
    char ch='\t';
    char prev;
    do
        {
            ch=fgetc(f1);
            if(ch=='\n')
                {
                    count=count+1;
                }
            prev=ch;
        }while(ch!=EOF);
    fclose(f1);
    fp=fopen(argv[1],"r");
    // use a for loop to go through the entire thing one by one, and read everything into the memory allocated1
    char directive[20];
    char size_str[20];
    int size;
    char address[20];
    fscanf(fp,"%s %s",directive,size_str);
    size=strtol(size_str,NULL,16);
    if(strcmp(directive,".size")==0)
        {
            IP=(char *)malloc(size);
        }
    else
        {
            printf("Size directive must be the first line of the program");
            exit(1);
        }
    char * start =IP;
    char input[500];
    int textfreq=0;
    int p=1;
    while(fscanf(fp,"%s %s %[^\n]s",directive,address,input)==3)
    {
    p=p+1;
    if(strcmp(directive,".string")==0)
        {
            string_dir(start,address,input);
        }
    else if(strcmp(directive,".long")==0)
        {
            long_dir(start,address,input);
        }
    else if(strcmp(directive,".byte")==0)
        {
            byte_dir(start,address,input);
        }
    else if(strcmp(directive,".text")==0)
        {
            textfreq=textfreq+1;
            text_dir(IP,address,input);
        }
    else if(strcmp(directive,"size")==0)
        {
            printf("There can be only one size directive.");
            exit(5);
        }
    else
        {
            printf("This directive is invalid, program exiting");
            exit(6);
        }
    }
    if(textfreq!=1)
        {
            printf("There must be one and exactly one text directive in the program.");
            exit(2);
        }
    bitfield_val * byte = (bitfield_val*) (&IP[initial]);
    while(!(byte->low==1 && byte->high==0))
        {
            byte = (bitfield_val*)&IP[initial];
            //printf("opcode %u%u \n",byte->low,byte->high);
            if(byte->low==0 && byte ->high==0)
                {
                    nop();
                    continue;
                }
            else if(byte->low==1 && byte -> high==0)
                {
                    halt(start);
                    break;
                }
            else if(byte->low==2 && byte ->high==0)
                {
                    rrmovl(IP,reg);
                    continue;
                }
            else if(byte->low==3 && byte->high==0)
                {
                    irmovl(IP,reg);
                    continue;
                }
            else if(byte->low==4 && byte->high==0)
                {
                    rmmovl(IP,reg,start,size);
                    continue;
                }
            else if(byte->low==5 && byte->high==0)
                {
                    mrmovl(IP,reg,start,size);
                    continue;
                }
            else if(byte->low==6)
                {
                    op1(IP,reg,flags);
                    continue;
                }
            else if(byte->low==7)
                {
                    jxx(IP,flags,start);
                    continue;
                }
            else if(byte->low==8 && byte->high==0)
                {
                    call(IP,reg,start,size);
                    continue;
                }
            else if(byte->low==9 && byte->high==0)
                {
                    ret(IP,reg,size);
                    continue;
                }
            else if(byte->low==10 && byte->high==0)
                {
                    pushl(IP,reg,size);
                    continue;
                }
            else if(byte->low==11 && byte->high==0)
                {
                    popl(IP,reg,size);
                    continue;
                }
            else if(byte->low==12 )
                {
                    read(IP,reg,flags,start);
                    continue;
                }
            else if(byte->low==13)
                {
                    write(IP,reg,start);
                    continue;
                }
            else if(byte->low==14 && byte->high==0)
                {
                    movsbl(IP,reg,start,size);
                    continue;
                }
            else
            {
                    fprintf(stderr,"invalid opcode entered into program");
                    exit(7);
                    break;
            }
        }
    fclose(fp);
        return 0;
}
