#include "spimcore.h"

//Nathan Warticki & Abdul Memon

/* ALU */
/* 10 Points */
void ALU(unsigned A, unsigned B, char ALUControl, unsigned *ALUresult, char *Zero)
{
  switch (ALUControl)
  {
  //Z = A + B
  case 0x0:
    *ALUresult = A + B;
    break;

  //Z = A - B
  case 0x1:
    *ALUresult = A - B;
    break;

  //if A < B, Z + 1, else Z = 0 (signed)
  case 0x2:
    if ((int)A < (int)B)
    {
      *ALUresult = 1;
    }
    else
    {
      *ALUresult = 0;
    }
    break;

  //if A < B, Z = 1, else Z = 0 (unsigned)
  case 0x3:
    if (A < B)
    {
      *ALUresult = 1;
    }
    else
    {
      *ALUresult = 0;
    }
    break;

  //Z = A && B
  case 0x4:
    *ALUresult = A & B;
    break;

  //Z = A || B
  case 0x5:
    *ALUresult = A | B;
    break;

  //shift B left by 16 bits
  case 0x6:
    *ALUresult = B << 16;
    break;

  //Z = !A
  case 0x7:
    *ALUresult = ~A;
    break;
  }

  //if result is 0, set Zero to 1, or 0 otherwise
  if (*ALUresult == 0)
  {
    *Zero = 1;
  }
  else
  {
    *Zero = 0;
  }
}

/* instruction fetch */
/* 10 Points */
int instruction_fetch(unsigned PC, unsigned *Mem, unsigned *instruction)
{
  if (PC % 4 != 0)
  {
    return 1;
  }

  *instruction = Mem[PC >> 2];
  return 0;
}

/* instruction partition */
/* 10 Points */
void instruction_partition(unsigned instruction, unsigned *op, unsigned *r1, unsigned *r2, unsigned *r3, unsigned *funct, unsigned *offset, unsigned *jsec)
{
  //instruction [31-26]
  *op = (instruction & 0xfc000000) >> 26;

  //instruction [25-21]
  *r1 = (instruction & 0x03e00000) >> 21;

  //instruction [20-16]
  *r2 = (instruction & 0x001f0000) >> 16;

  //instruction [15-11]
  *r3 = (instruction & 0x0000f800) >> 11;

  //instruction [5-0]
  *funct = instruction & 0x0000003f;

  //instruction [15-0]
  *offset = instruction & 0x0000ffff;

  //instruction [25-0]
  *jsec = instruction & 0x03ffffff;
}

/* instruction decode */
/* 15 Points */
int instruction_decode(unsigned op, struct_controls *controls)
{
  //start at 0
  controls->RegDst = 0;
  controls->Jump = 0;
  controls->Branch = 0;
  controls->MemRead = 0;
  controls->MemtoReg = 0;
  controls->ALUOp = 0;
  controls->MemWrite = 0;
  controls->ALUSrc = 0;
  controls->RegWrite = 0;

  switch (op)
  {
  //R-Types
  case 0x0:
    controls->RegDst = 1;
    controls->ALUOp = 7;
    controls->RegWrite = 1;
    break;

  //add immediate
  case 0x8:
    controls->RegWrite = 1;
    controls->ALUSrc = 1;
    break;

  //load word
  case 0x23:
    controls->RegWrite = 1;
    controls->MemRead = 1;
    controls->MemtoReg = 1;
    controls->ALUSrc = 1;
    break;

  //store word
  case 0x2b:
    controls->MemWrite = 1;
    controls->RegDst = 2;   // ?
    controls->MemtoReg = 2; // ?
    controls->ALUSrc = 1;
    break;

  //load upper immediate
  case 0xf:
    controls->RegWrite = 1;
    //use first 16 bits
    controls->ALUOp = 6;
    controls->ALUSrc = 1;
    break;

  //branch on equal
  case 0x4:
    //PC updates
    controls->Branch = 1;
    controls->RegDst = 2;
    controls->MemtoReg = 2;
    controls->ALUSrc = 0;
    //subtraction
    controls->ALUOp = 1;
    break;

  //set less than immediate
  case 0xa:
    //set op
    controls->ALUOp = 2;
    controls->RegWrite = 1;
    controls->ALUSrc = 1;
    break;

  //set less than immediate unsigned=
  case 0xb:
    controls->ALUOp = 3;
    controls->RegWrite = 1;
    controls->ALUSrc = 1;
    break;

  //jumps
  case 0x2:
    controls->Jump = 1;
    controls->RegDst = 2;
    controls->Branch = 2;
    controls->MemtoReg = 2;
    controls->ALUSrc = 2;
    controls->ALUOp = 2;
    break;

  default:
    return 1;
  }
  return 0;
}

/* Read Register */
/* 5 Points */
void read_register(unsigned r1, unsigned r2, unsigned *Reg, unsigned *data1, unsigned *data2)
{
  //retrieves r1 and 2 values, stores them in data 1 and 2
  *data1 = Reg[r1];
  *data2 = Reg[r2];
}

/* Sign Extend */
/* 10 Points */
void sign_extend(unsigned offset, unsigned *extended_value)
{
  if ((offset >> 15) == 1)
  {
    //fill with 1's when -
    *extended_value = offset | 0xffff0000;
  }
  else
  {
    //fill with 0's when +
    *extended_value = offset & 0x0000ffff;
  }
}

/* ALU operations */
/* 10 Points */
int ALU_operations(unsigned data1, unsigned data2, unsigned extended_value, unsigned funct, char ALUOp, char ALUSrc, unsigned *ALUresult, char *Zero)
{
  unsigned aluControl = ALUOp;

  //checks for bad positions
  if (ALUOp < 0 || ALUOp > 7)
  {
    return 1;
  }

  //if op is R-type
  if (ALUOp == 7)
  {
    switch (funct)
    {
    //addition case
    case 0x20:
      aluControl = 0;
      break;
    //subtraction case
    case 0x22:
      aluControl = 1;
      break;
    //and Comparison Case
    case 0x24:
      aluControl = 4;
      break;
    //or Comparison Case
    case 0x25:
      aluControl = 5;
      break;
    //< case
    case 0x2a:
      aluControl = 2;
      break;
    //< unsigned case
    case 0x2b:
      aluControl = 3;
      break;
    //default case for invalid instructions
    default:
      return 1;
    }
  }

  unsigned second = (ALUSrc == 1) ? extended_value : data2;
  ALU(data1, second, aluControl, ALUresult, Zero);

  return 0;
}

/* Read / Write Memory */
/* 10 Points */
int rw_memory(unsigned ALUresult, unsigned data2, char MemWrite, char MemRead, unsigned *memdata, unsigned *Mem)
{
  //puts data2 value in memory
  if (MemWrite == 1)
  {
    if (ALUresult % 4 == 0)
    {
      Mem[ALUresult >> 2] = data2;
    }
    else
    {
      return 1;
    }
  }

  //result content read to memdata
  if (MemRead == 1)
  {
    if (ALUresult % 4 == 0)
    {
      *memdata = Mem[ALUresult >> 2];
    }
    else
    {
      return 1;
    }
  }

  return 0;
}

/* Write Register */
/* 10 Points */
void write_register(unsigned r2, unsigned r3, unsigned memdata, unsigned ALUresult, char RegWrite, char RegDst, char MemtoReg, unsigned *Reg)
{
  if (RegWrite == 1)
  {
    //determines if instruction from memory is I-type
    if (MemtoReg == 1)
    {
      Reg[r2] = memdata;
    }

    //identifies which instruction type came from register
    else if (MemtoReg == 0)
    {
      //write R-type
      if (RegDst == 1)
      {
        Reg[r3] = ALUresult;
      }

      //write I-type
      else
      {
        Reg[r2] = ALUresult;
      }
    }
  }
}

/* PC update */
/* 10 Points */
void PC_update(unsigned jsec, unsigned extended_value, char Branch, char Jump, char Zero, unsigned *PC)
{
  *PC += 4;

  //ALU sends signal of 0 and the branch is taken, increments *PC
  if (Branch == 1 && Zero == 1)
  {
    *PC += (extended_value << 2);
  }

  //shifts PC with jsec bits, only if call to jump
  if (Jump == 1)
  {
    *PC = (*PC & 0xf000000) | (jsec << 2);
  }
}
