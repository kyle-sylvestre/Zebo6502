void op_inv_imp(Sim6502 &sim){ printf("ILLEGAL OPCODE\n");init_gui_debugging(sim); sim.exec_mode = ExecMode_Pause; }
const Opcode OPCODE_ILLEGAL = {"???", op_inv_imp, AddrMode_Invalid };

void
op_brk_imp(Sim6502 &sim)
{
    switch (sim.cycle) {
        case 2: {
            sim.tmp16 = sim.pc + 2;
             /* discard read PC + 1 */
        } break;
        case 3: {
            push_stack(sim, READ_HI(sim.tmp16));
        } break;
        case 4: {
            push_stack(sim, READ_LO(sim.tmp16));
        } break;
        case 5: {
            sim.pflags.brk = 1;
            push_stack(sim, sim.pflags.data | 0x20); // bit 5 always high
        } break;
        case 6: {
            sim.pflags.intdis = 1;
            WRITE_LO(sim.pc, mem_read(sim, 0xFFFE));
        } break;
        case 7: {
            WRITE_HI(sim.pc, mem_read(sim, 0xFFFF));
            sim.cycle = 1;
        } break;
        
    }
}
void
op_ora_izx(Sim6502 &sim)
{
    switch (sim.cycle) {
        case 2: {
            sim.tmp16 = mem_read(sim, sim.pc + 1);
        } break;
        case 3: {
            sim.tmp16 = (sim.tmp16 + sim.x) & 0xFF;
        } break;
        case 4: {
            sim.tmp8 = mem_read(sim, sim.tmp16 + 0);
        } break;
        case 5: {
            WRITE_HI(sim.tmp16, mem_read(sim, sim.tmp16 + 1));
        } break;
        case 6: {
            WRITE_LO(sim.tmp16, sim.tmp8);
        } break;
        case 7: {
            sim.tmp8 = mem_read(sim, sim.tmp16);
            sim.acc = sim.acc | sim.tmp8;
            setnz(sim, sim.acc);
            sim.cycle = 1;
            sim.pc += 2;
        } break;
        
    }
}
void
op_ora_zp(Sim6502 &sim)
{
    switch (sim.cycle) {
        case 2: {
            sim.tmp16 = ((uint16)mem_read(sim, sim.pc + 1));
        } break;
        case 3: {
            sim.tmp8 = mem_read(sim, sim.tmp16);
        } break;
        case 4: {
            sim.acc = sim.acc | sim.tmp8;
            setnz(sim, sim.acc);
            sim.cycle = 1;
            sim.pc += 2;
        } break;
        
    }
}
void
op_asl_zp(Sim6502 &sim)
{
    switch (sim.cycle) {
        case 2: {
            sim.tmp16 = ((uint16)mem_read(sim, sim.pc + 1));
        } break;
        case 3: {
            sim.tmp8 = mem_read(sim, sim.tmp16);
        } break;
        case 4: {
             /* discard read memwrite ADL */
        } break;
        case 5: {
            sim.pflags.carry = (sim.tmp8 & 0x80) != 0;
            sim.tmp8 = sim.tmp8 << 1;
            setnz(sim, sim.tmp8);
            mem_write(sim, sim.tmp16, sim.tmp8);
            sim.cycle = 1;
            sim.pc += 2;
        } break;
        
    }
}
void
op_php_imp(Sim6502 &sim)
{
    switch (sim.cycle) {
        case 2: {
             /* discard read PC + 1 */
        } break;
        case 3: {
            sim.pflags.brk = 1;
            push_stack(sim, sim.pflags.data | 0x20); // bit 5 always high
            sim.pflags.brk = 0;
            sim.cycle = 1;
            sim.pc += 1;
        } break;
        
    }
}
void
op_ora_imm(Sim6502 &sim)
{
    switch (sim.cycle) {
        case 2: {
            sim.tmp8 = mem_read(sim, sim.pc + 1);
        } break;
        case 3: {
            sim.acc = sim.acc | sim.tmp8;
            setnz(sim, sim.acc);
            sim.cycle = 1;
            sim.pc += 2;
        } break;
        
    }
}
void
op_asl_acc(Sim6502 &sim)
{
    switch (sim.cycle) {
        case 2: {
            sim.pflags.carry = (sim.acc & 0x80) != 0;
            sim.acc = sim.acc << 1;
            setnz(sim, sim.acc);
            sim.cycle = 1;
            sim.pc += 1;
        } break;
        
    }
}
void
op_ora_abs(Sim6502 &sim)
{
    switch (sim.cycle) {
        case 2: {
            WRITE_LO(sim.tmp16, mem_read(sim, sim.pc + 1));
        } break;
        case 3: {
            WRITE_HI(sim.tmp16, mem_read(sim, sim.pc + 2));
        } break;
        case 4: {
            sim.tmp8 = mem_read(sim, sim.tmp16 + ((uint16)0));
        } break;
        case 5: {
            sim.acc = sim.acc | sim.tmp8;
            setnz(sim, sim.acc);
            sim.cycle = 1;
            sim.pc += 3;
        } break;
        
    }
}
void
op_asl_abs(Sim6502 &sim)
{
    switch (sim.cycle) {
        case 2: {
            WRITE_LO(sim.tmp16, mem_read(sim, sim.pc + 1));
        } break;
        case 3: {
            WRITE_HI(sim.tmp16, mem_read(sim, sim.pc + 2));
        } break;
        case 4: {
            sim.tmp8 = mem_read(sim, sim.tmp16 + ((uint16)0));
        } break;
        case 5: {
             /* discard unmodified write */
        } break;
        case 6: {
            sim.pflags.carry = (sim.tmp8 & 0x80) != 0;
            sim.tmp8 = sim.tmp8 << 1;
            setnz(sim, sim.tmp8);
            mem_write(sim, sim.tmp16 + ((uint16)0), sim.tmp8);
            sim.cycle = 1;
            sim.pc += 3;
        } break;
        
    }
}
void
op_bpl_rel(Sim6502 &sim)
{
    switch (sim.cycle) {
        case 2: {
            // test the opposite of the operation
            if ( !(sim.pflags.neg == 0) ) { sim.cycle = 1; sim.pc += 2; }
            
            // break on test methods that sucessfully branch to itself
            else if (mem_read(sim, sim.pc + 1) == 0xFE) { init_gui_debugging(sim); sim.exec_mode = ExecMode_Pause; }
        } break;
        case 3: {
            sim.tmp16 = mem_read(sim, sim.pc + 1);
            if (sim.tmp16 & 0x80) sim.tmp16 |= 0xFF00; // sign extend
            sim.tmp16 += sim.pc + 2;
            bool overflow = READ_HI(sim.pc + 2) != READ_HI(sim.tmp16);
            sim.pc = sim.tmp16;
            if (!overflow) { sim.cycle = 1; } // page boundary cross check
        } break;
        case 4: {
             /* discard increment high byte of T16 */
            sim.cycle = 1;
        } break;
        
    }
}
void
op_ora_izy(Sim6502 &sim)
{
    switch (sim.cycle) {
        case 2: {
            sim.tmp16 = mem_read(sim, sim.pc + 1);
        } break;
        case 3: {
            sim.tmp8 = mem_read(sim, sim.tmp16 + 0);
        } break;
        case 4: {
            WRITE_HI(sim.tmp16, mem_read(sim, sim.tmp16 + 1));
        } break;
        case 5: {
            WRITE_LO(sim.tmp16, sim.tmp8);
        } break;
        case 6: {
            sim.tmp16 += sim.y;
        } break;
        case 7: {
            sim.tmp8 = mem_read(sim, sim.tmp16);
            sim.acc = sim.acc | sim.tmp8;
            setnz(sim, sim.acc);
            sim.cycle = 1;
            sim.pc += 2;
        } break;
        
    }
}
void
op_ora_zpx(Sim6502 &sim)
{
    switch (sim.cycle) {
        case 2: {
            sim.tmp8 = mem_read(sim, sim.pc + 1);
        } break;
        case 3: {
             /* discard memread BAL */
        } break;
        case 4: {
            sim.tmp16 = (sim.x + sim.tmp8) & 0xFF;
        } break;
        case 5: {
            sim.tmp8 = mem_read(sim, sim.tmp16);
        } break;
        case 6: {
            sim.acc = sim.acc | sim.tmp8;
            setnz(sim, sim.acc);
            sim.cycle = 1;
            sim.pc += 2;
        } break;
        
    }
}
void
op_asl_zpx(Sim6502 &sim)
{
    switch (sim.cycle) {
        case 2: {
            sim.tmp8 = mem_read(sim, sim.pc + 1);
        } break;
        case 3: {
             /* discard memread BAL */
        } break;
        case 4: {
            sim.tmp16 = (sim.x + sim.tmp8) & 0xFF;
        } break;
        case 5: {
            sim.tmp8 = mem_read(sim, sim.tmp16);
        } break;
        case 6: {
             /* discard unmodified write */
        } break;
        case 7: {
            sim.pflags.carry = (sim.tmp8 & 0x80) != 0;
            sim.tmp8 = sim.tmp8 << 1;
            setnz(sim, sim.tmp8);
            mem_write(sim, sim.tmp16, sim.tmp8);
            sim.cycle = 1;
            sim.pc += 2;
        } break;
        
    }
}
void
op_clc_imp(Sim6502 &sim)
{
    switch (sim.cycle) {
        case 2: {
            sim.pflags.carry = 0;
            sim.cycle = 1;
            sim.pc += 1;
        } break;
        
    }
}
void
op_ora_absy(Sim6502 &sim)
{
    switch (sim.cycle) {
        case 2: {
            WRITE_LO(sim.tmp16, mem_read(sim, sim.pc + 1));
        } break;
        case 3: {
            WRITE_HI(sim.tmp16, mem_read(sim, sim.pc + 2));
        } break;
        case 4: {
            sim.tmp8 = mem_read(sim, sim.tmp16 + ((uint16)sim.y));
        } break;
        case 5: {
            sim.acc = sim.acc | sim.tmp8;
            setnz(sim, sim.acc);
            sim.cycle = 1;
            sim.pc += 3;
        } break;
        
    }
}
void
op_ora_absx(Sim6502 &sim)
{
    switch (sim.cycle) {
        case 2: {
            WRITE_LO(sim.tmp16, mem_read(sim, sim.pc + 1));
        } break;
        case 3: {
            WRITE_HI(sim.tmp16, mem_read(sim, sim.pc + 2));
        } break;
        case 4: {
            sim.tmp8 = mem_read(sim, sim.tmp16 + ((uint16)sim.x));
        } break;
        case 5: {
            sim.acc = sim.acc | sim.tmp8;
            setnz(sim, sim.acc);
            sim.cycle = 1;
            sim.pc += 3;
        } break;
        
    }
}
void
op_asl_absx(Sim6502 &sim)
{
    switch (sim.cycle) {
        case 2: {
            WRITE_LO(sim.tmp16, mem_read(sim, sim.pc + 1));
        } break;
        case 3: {
            WRITE_HI(sim.tmp16, mem_read(sim, sim.pc + 2));
        } break;
        case 4: {
             /* discard read */
        } break;
        case 5: {
            sim.tmp8 = mem_read(sim, sim.tmp16 + ((uint16)sim.x));
        } break;
        case 6: {
             /* discard unmodified write */
        } break;
        case 7: {
            sim.pflags.carry = (sim.tmp8 & 0x80) != 0;
            sim.tmp8 = sim.tmp8 << 1;
            setnz(sim, sim.tmp8);
            mem_write(sim, sim.tmp16 + ((uint16)sim.x), sim.tmp8);
            sim.cycle = 1;
            sim.pc += 3;
        } break;
        
    }
}
void
op_jsr_abs(Sim6502 &sim)
{
    switch (sim.cycle) {
        case 2: {
            WRITE_LO(sim.tmp16, mem_read(sim, sim.pc + 1));
        } break;
        case 3: {
             /* discard read SP */
        } break;
        case 4: {
            push_stack(sim, READ_HI(sim.pc + 2));
        } break;
        case 5: {
            push_stack(sim, READ_LO(sim.pc + 2));
        } break;
        case 6: {
            WRITE_HI(sim.tmp16, mem_read(sim, sim.pc + 2));
        } break;
        case 7: {
            sim.pc = sim.tmp16;
            sim.cycle = 1;
        } break;
        
    }
}
void
op_and_izx(Sim6502 &sim)
{
    switch (sim.cycle) {
        case 2: {
            sim.tmp16 = mem_read(sim, sim.pc + 1);
        } break;
        case 3: {
            sim.tmp16 = (sim.tmp16 + sim.x) & 0xFF;
        } break;
        case 4: {
            sim.tmp8 = mem_read(sim, sim.tmp16 + 0);
        } break;
        case 5: {
            WRITE_HI(sim.tmp16, mem_read(sim, sim.tmp16 + 1));
        } break;
        case 6: {
            WRITE_LO(sim.tmp16, sim.tmp8);
        } break;
        case 7: {
            sim.tmp8 = mem_read(sim, sim.tmp16);
            sim.acc = sim.acc & sim.tmp8;
            setnz(sim, sim.acc);
            sim.cycle = 1;
            sim.pc += 2;
        } break;
        
    }
}
void
op_bit_zp(Sim6502 &sim)
{
    switch (sim.cycle) {
        case 2: {
            sim.tmp16 = ((uint16)mem_read(sim, sim.pc + 1));
        } break;
        case 3: {
            sim.tmp8 = mem_read(sim, sim.tmp16);
        } break;
        case 4: {
            sim.pflags.zero = (sim.acc & sim.tmp8) == 0;
            sim.pflags.ovrf = (sim.tmp8 & 0x40) != 0;
            sim.pflags.neg = (sim.tmp8 & 0x80) != 0;
            sim.cycle = 1;
            sim.pc += 2;
        } break;
        
    }
}
void
op_and_zp(Sim6502 &sim)
{
    switch (sim.cycle) {
        case 2: {
            sim.tmp16 = ((uint16)mem_read(sim, sim.pc + 1));
        } break;
        case 3: {
            sim.tmp8 = mem_read(sim, sim.tmp16);
        } break;
        case 4: {
            sim.acc = sim.acc & sim.tmp8;
            setnz(sim, sim.acc);
            sim.cycle = 1;
            sim.pc += 2;
        } break;
        
    }
}
void
op_rol_zp(Sim6502 &sim)
{
    switch (sim.cycle) {
        case 2: {
            sim.tmp16 = ((uint16)mem_read(sim, sim.pc + 1));
        } break;
        case 3: {
            sim.tmp8 = mem_read(sim, sim.tmp16);
        } break;
        case 4: {
             /* discard read memwrite ADL */
        } break;
        case 5: {
            uint8 bit7 = (sim.tmp8 & 0x80) != 0;
            sim.tmp8 = (sim.tmp8 << 1) | ((uint8)sim.pflags.carry);
            sim.pflags.carry = bit7;
            setnz(sim, sim.tmp8);
            mem_write(sim, sim.tmp16, sim.tmp8);
            sim.cycle = 1;
            sim.pc += 2;
        } break;
        
    }
}
void
op_plp_imp(Sim6502 &sim)
{
    switch (sim.cycle) {
        case 2: {
             /* discard read PC + 1 */
        } break;
        case 3: {
             /* discard read SP */
        } break;
        case 4: {
            sim.pflags.data = pop_stack(sim);
            sim.cycle = 1;
            sim.pc += 1;
        } break;
        
    }
}
void
op_and_imm(Sim6502 &sim)
{
    switch (sim.cycle) {
        case 2: {
            sim.tmp8 = mem_read(sim, sim.pc + 1);
        } break;
        case 3: {
            sim.acc = sim.acc & sim.tmp8;
            setnz(sim, sim.acc);
            sim.cycle = 1;
            sim.pc += 2;
        } break;
        
    }
}
void
op_rol_acc(Sim6502 &sim)
{
    switch (sim.cycle) {
        case 2: {
            uint8 bit7 = (sim.tmp8 & 0x80) != 0;
            sim.acc = (sim.acc << 1) | ((uint8)sim.pflags.carry);
            sim.pflags.carry = bit7;
            setnz(sim, sim.acc);
            sim.cycle = 1;
            sim.pc += 1;
        } break;
        
    }
}
void
op_bit_abs(Sim6502 &sim)
{
    switch (sim.cycle) {
        case 2: {
            WRITE_LO(sim.tmp16, mem_read(sim, sim.pc + 1));
        } break;
        case 3: {
            WRITE_HI(sim.tmp16, mem_read(sim, sim.pc + 2));
        } break;
        case 4: {
            sim.tmp8 = mem_read(sim, sim.tmp16 + ((uint16)0));
        } break;
        case 5: {
            sim.pflags.zero = (sim.acc & sim.tmp8) == 0;
            sim.pflags.ovrf = (sim.tmp8 & 0x40) != 0;
            sim.pflags.neg = (sim.tmp8 & 0x80) != 0;
            sim.cycle = 1;
            sim.pc += 3;
        } break;
        
    }
}
void
op_and_abs(Sim6502 &sim)
{
    switch (sim.cycle) {
        case 2: {
            WRITE_LO(sim.tmp16, mem_read(sim, sim.pc + 1));
        } break;
        case 3: {
            WRITE_HI(sim.tmp16, mem_read(sim, sim.pc + 2));
        } break;
        case 4: {
            sim.tmp8 = mem_read(sim, sim.tmp16 + ((uint16)0));
        } break;
        case 5: {
            sim.acc = sim.acc & sim.tmp8;
            setnz(sim, sim.acc);
            sim.cycle = 1;
            sim.pc += 3;
        } break;
        
    }
}
void
op_rol_abs(Sim6502 &sim)
{
    switch (sim.cycle) {
        case 2: {
            WRITE_LO(sim.tmp16, mem_read(sim, sim.pc + 1));
        } break;
        case 3: {
            WRITE_HI(sim.tmp16, mem_read(sim, sim.pc + 2));
        } break;
        case 4: {
            sim.tmp8 = mem_read(sim, sim.tmp16 + ((uint16)0));
        } break;
        case 5: {
             /* discard unmodified write */
        } break;
        case 6: {
            uint8 bit7 = (sim.tmp8 & 0x80) != 0;
            sim.tmp8 = (sim.tmp8 << 1) | ((uint8)sim.pflags.carry);
            sim.pflags.carry = bit7;
            setnz(sim, sim.tmp8);
            mem_write(sim, sim.tmp16 + ((uint16)0), sim.tmp8);
            sim.cycle = 1;
            sim.pc += 3;
        } break;
        
    }
}
void
op_bmi_rel(Sim6502 &sim)
{
    switch (sim.cycle) {
        case 2: {
            // test the opposite of the operation
            if ( !(sim.pflags.neg == 1) ) { sim.cycle = 1; sim.pc += 2; }
            
            // break on test methods that sucessfully branch to itself
            else if (mem_read(sim, sim.pc + 1) == 0xFE) { init_gui_debugging(sim); sim.exec_mode = ExecMode_Pause; }
        } break;
        case 3: {
            sim.tmp16 = mem_read(sim, sim.pc + 1);
            if (sim.tmp16 & 0x80) sim.tmp16 |= 0xFF00; // sign extend
            sim.tmp16 += sim.pc + 2;
            bool overflow = READ_HI(sim.pc + 2) != READ_HI(sim.tmp16);
            sim.pc = sim.tmp16;
            if (!overflow) { sim.cycle = 1; } // page boundary cross check
        } break;
        case 4: {
             /* discard increment high byte of T16 */
            sim.cycle = 1;
        } break;
        
    }
}
void
op_and_izy(Sim6502 &sim)
{
    switch (sim.cycle) {
        case 2: {
            sim.tmp16 = mem_read(sim, sim.pc + 1);
        } break;
        case 3: {
            sim.tmp8 = mem_read(sim, sim.tmp16 + 0);
        } break;
        case 4: {
            WRITE_HI(sim.tmp16, mem_read(sim, sim.tmp16 + 1));
        } break;
        case 5: {
            WRITE_LO(sim.tmp16, sim.tmp8);
        } break;
        case 6: {
            sim.tmp16 += sim.y;
        } break;
        case 7: {
            sim.tmp8 = mem_read(sim, sim.tmp16);
            sim.acc = sim.acc & sim.tmp8;
            setnz(sim, sim.acc);
            sim.cycle = 1;
            sim.pc += 2;
        } break;
        
    }
}
void
op_and_zpx(Sim6502 &sim)
{
    switch (sim.cycle) {
        case 2: {
            sim.tmp8 = mem_read(sim, sim.pc + 1);
        } break;
        case 3: {
             /* discard memread BAL */
        } break;
        case 4: {
            sim.tmp16 = (sim.x + sim.tmp8) & 0xFF;
        } break;
        case 5: {
            sim.tmp8 = mem_read(sim, sim.tmp16);
        } break;
        case 6: {
            sim.acc = sim.acc & sim.tmp8;
            setnz(sim, sim.acc);
            sim.cycle = 1;
            sim.pc += 2;
        } break;
        
    }
}
void
op_rol_zpx(Sim6502 &sim)
{
    switch (sim.cycle) {
        case 2: {
            sim.tmp8 = mem_read(sim, sim.pc + 1);
        } break;
        case 3: {
             /* discard memread BAL */
        } break;
        case 4: {
            sim.tmp16 = (sim.x + sim.tmp8) & 0xFF;
        } break;
        case 5: {
            sim.tmp8 = mem_read(sim, sim.tmp16);
        } break;
        case 6: {
             /* discard unmodified write */
        } break;
        case 7: {
            uint8 bit7 = (sim.tmp8 & 0x80) != 0;
            sim.tmp8 = (sim.tmp8 << 1) | ((uint8)sim.pflags.carry);
            sim.pflags.carry = bit7;
            setnz(sim, sim.tmp8);
            mem_write(sim, sim.tmp16, sim.tmp8);
            sim.cycle = 1;
            sim.pc += 2;
        } break;
        
    }
}
void
op_sec_imp(Sim6502 &sim)
{
    switch (sim.cycle) {
        case 2: {
            sim.pflags.carry = 1;
            sim.cycle = 1;
            sim.pc += 1;
        } break;
        
    }
}
void
op_and_absy(Sim6502 &sim)
{
    switch (sim.cycle) {
        case 2: {
            WRITE_LO(sim.tmp16, mem_read(sim, sim.pc + 1));
        } break;
        case 3: {
            WRITE_HI(sim.tmp16, mem_read(sim, sim.pc + 2));
        } break;
        case 4: {
            sim.tmp8 = mem_read(sim, sim.tmp16 + ((uint16)sim.y));
        } break;
        case 5: {
            sim.acc = sim.acc & sim.tmp8;
            setnz(sim, sim.acc);
            sim.cycle = 1;
            sim.pc += 3;
        } break;
        
    }
}
void
op_and_absx(Sim6502 &sim)
{
    switch (sim.cycle) {
        case 2: {
            WRITE_LO(sim.tmp16, mem_read(sim, sim.pc + 1));
        } break;
        case 3: {
            WRITE_HI(sim.tmp16, mem_read(sim, sim.pc + 2));
        } break;
        case 4: {
            sim.tmp8 = mem_read(sim, sim.tmp16 + ((uint16)sim.x));
        } break;
        case 5: {
            sim.acc = sim.acc & sim.tmp8;
            setnz(sim, sim.acc);
            sim.cycle = 1;
            sim.pc += 3;
        } break;
        
    }
}
void
op_rol_absx(Sim6502 &sim)
{
    switch (sim.cycle) {
        case 2: {
            WRITE_LO(sim.tmp16, mem_read(sim, sim.pc + 1));
        } break;
        case 3: {
            WRITE_HI(sim.tmp16, mem_read(sim, sim.pc + 2));
        } break;
        case 4: {
             /* discard read */
        } break;
        case 5: {
            sim.tmp8 = mem_read(sim, sim.tmp16 + ((uint16)sim.x));
        } break;
        case 6: {
             /* discard unmodified write */
        } break;
        case 7: {
            uint8 bit7 = (sim.tmp8 & 0x80) != 0;
            sim.tmp8 = (sim.tmp8 << 1) | ((uint8)sim.pflags.carry);
            sim.pflags.carry = bit7;
            setnz(sim, sim.tmp8);
            mem_write(sim, sim.tmp16 + ((uint16)sim.x), sim.tmp8);
            sim.cycle = 1;
            sim.pc += 3;
        } break;
        
    }
}
void
op_rti_imp(Sim6502 &sim)
{
    switch (sim.cycle) {
        case 2: {
             /* discard read PC + 1 */
        } break;
        case 3: {
             /* discard read SP */
        } break;
        case 4: {
            sim.pflags.data = pop_stack(sim);
        } break;
        case 5: {
            WRITE_LO(sim.pc, pop_stack(sim));
        } break;
        case 6: {
            WRITE_HI(sim.pc, pop_stack(sim));
            sim.cycle = 1;
        } break;
        
    }
}
void
op_eor_izx(Sim6502 &sim)
{
    switch (sim.cycle) {
        case 2: {
            sim.tmp16 = mem_read(sim, sim.pc + 1);
        } break;
        case 3: {
            sim.tmp16 = (sim.tmp16 + sim.x) & 0xFF;
        } break;
        case 4: {
            sim.tmp8 = mem_read(sim, sim.tmp16 + 0);
        } break;
        case 5: {
            WRITE_HI(sim.tmp16, mem_read(sim, sim.tmp16 + 1));
        } break;
        case 6: {
            WRITE_LO(sim.tmp16, sim.tmp8);
        } break;
        case 7: {
            sim.tmp8 = mem_read(sim, sim.tmp16);
            sim.acc = sim.acc ^ sim.tmp8;
            setnz(sim, sim.acc);
            sim.cycle = 1;
            sim.pc += 2;
        } break;
        
    }
}
void
op_eor_zp(Sim6502 &sim)
{
    switch (sim.cycle) {
        case 2: {
            sim.tmp16 = ((uint16)mem_read(sim, sim.pc + 1));
        } break;
        case 3: {
            sim.tmp8 = mem_read(sim, sim.tmp16);
        } break;
        case 4: {
            sim.acc = sim.acc ^ sim.tmp8;
            setnz(sim, sim.acc);
            sim.cycle = 1;
            sim.pc += 2;
        } break;
        
    }
}
void
op_lsr_zp(Sim6502 &sim)
{
    switch (sim.cycle) {
        case 2: {
            sim.tmp16 = ((uint16)mem_read(sim, sim.pc + 1));
        } break;
        case 3: {
            sim.tmp8 = mem_read(sim, sim.tmp16);
        } break;
        case 4: {
             /* discard read memwrite ADL */
        } break;
        case 5: {
            sim.pflags.carry = (sim.tmp8 & 0x01) != 0;
            sim.tmp8 = sim.tmp8 >> 1;
            setnz(sim, sim.tmp8);
            mem_write(sim, sim.tmp16, sim.tmp8);
            sim.cycle = 1;
            sim.pc += 2;
        } break;
        
    }
}
void
op_pha_imp(Sim6502 &sim)
{
    switch (sim.cycle) {
        case 2: {
             /* discard read PC + 1 */
        } break;
        case 3: {
            push_stack(sim, sim.acc);
            sim.cycle = 1;
            sim.pc += 1;
        } break;
        
    }
}
void
op_eor_imm(Sim6502 &sim)
{
    switch (sim.cycle) {
        case 2: {
            sim.tmp8 = mem_read(sim, sim.pc + 1);
        } break;
        case 3: {
            sim.acc = sim.acc ^ sim.tmp8;
            setnz(sim, sim.acc);
            sim.cycle = 1;
            sim.pc += 2;
        } break;
        
    }
}
void
op_lsr_acc(Sim6502 &sim)
{
    switch (sim.cycle) {
        case 2: {
            sim.pflags.carry = (sim.acc & 0x01) != 0;
            sim.acc = sim.acc >> 1;
            setnz(sim, sim.acc);
            sim.cycle = 1;
            sim.pc += 1;
        } break;
        
    }
}
void
op_jmp_abs(Sim6502 &sim)
{
    switch (sim.cycle) {
        case 2: {
            WRITE_LO(sim.tmp16, mem_read(sim, sim.pc + 1));
        } break;
        case 3: {
            WRITE_HI(sim.tmp16, mem_read(sim, sim.pc + 2));
        } break;
        case 4: {
            if (sim.tmp16 == sim.pc) { init_gui_debugging(sim); sim.exec_mode = ExecMode_Pause; } /* self jmp break */
        } break;
        case 5: {
            sim.pc = sim.tmp16;
            sim.cycle = 1;
        } break;
        
    }
}
void
op_eor_abs(Sim6502 &sim)
{
    switch (sim.cycle) {
        case 2: {
            WRITE_LO(sim.tmp16, mem_read(sim, sim.pc + 1));
        } break;
        case 3: {
            WRITE_HI(sim.tmp16, mem_read(sim, sim.pc + 2));
        } break;
        case 4: {
            sim.tmp8 = mem_read(sim, sim.tmp16 + ((uint16)0));
        } break;
        case 5: {
            sim.acc = sim.acc ^ sim.tmp8;
            setnz(sim, sim.acc);
            sim.cycle = 1;
            sim.pc += 3;
        } break;
        
    }
}
void
op_lsr_abs(Sim6502 &sim)
{
    switch (sim.cycle) {
        case 2: {
            WRITE_LO(sim.tmp16, mem_read(sim, sim.pc + 1));
        } break;
        case 3: {
            WRITE_HI(sim.tmp16, mem_read(sim, sim.pc + 2));
        } break;
        case 4: {
            sim.tmp8 = mem_read(sim, sim.tmp16 + ((uint16)0));
        } break;
        case 5: {
             /* discard unmodified write */
        } break;
        case 6: {
            sim.pflags.carry = (sim.tmp8 & 0x01) != 0;
            sim.tmp8 = sim.tmp8 >> 1;
            setnz(sim, sim.tmp8);
            mem_write(sim, sim.tmp16 + ((uint16)0), sim.tmp8);
            sim.cycle = 1;
            sim.pc += 3;
        } break;
        
    }
}
void
op_bvc_rel(Sim6502 &sim)
{
    switch (sim.cycle) {
        case 2: {
            // test the opposite of the operation
            if ( !(sim.pflags.ovrf == 0) ) { sim.cycle = 1; sim.pc += 2; }
            
            // break on test methods that sucessfully branch to itself
            else if (mem_read(sim, sim.pc + 1) == 0xFE) { init_gui_debugging(sim); sim.exec_mode = ExecMode_Pause; }
        } break;
        case 3: {
            sim.tmp16 = mem_read(sim, sim.pc + 1);
            if (sim.tmp16 & 0x80) sim.tmp16 |= 0xFF00; // sign extend
            sim.tmp16 += sim.pc + 2;
            bool overflow = READ_HI(sim.pc + 2) != READ_HI(sim.tmp16);
            sim.pc = sim.tmp16;
            if (!overflow) { sim.cycle = 1; } // page boundary cross check
        } break;
        case 4: {
             /* discard increment high byte of T16 */
            sim.cycle = 1;
        } break;
        
    }
}
void
op_eor_izy(Sim6502 &sim)
{
    switch (sim.cycle) {
        case 2: {
            sim.tmp16 = mem_read(sim, sim.pc + 1);
        } break;
        case 3: {
            sim.tmp8 = mem_read(sim, sim.tmp16 + 0);
        } break;
        case 4: {
            WRITE_HI(sim.tmp16, mem_read(sim, sim.tmp16 + 1));
        } break;
        case 5: {
            WRITE_LO(sim.tmp16, sim.tmp8);
        } break;
        case 6: {
            sim.tmp16 += sim.y;
        } break;
        case 7: {
            sim.tmp8 = mem_read(sim, sim.tmp16);
            sim.acc = sim.acc ^ sim.tmp8;
            setnz(sim, sim.acc);
            sim.cycle = 1;
            sim.pc += 2;
        } break;
        
    }
}
void
op_eor_zpx(Sim6502 &sim)
{
    switch (sim.cycle) {
        case 2: {
            sim.tmp8 = mem_read(sim, sim.pc + 1);
        } break;
        case 3: {
             /* discard memread BAL */
        } break;
        case 4: {
            sim.tmp16 = (sim.x + sim.tmp8) & 0xFF;
        } break;
        case 5: {
            sim.tmp8 = mem_read(sim, sim.tmp16);
        } break;
        case 6: {
            sim.acc = sim.acc ^ sim.tmp8;
            setnz(sim, sim.acc);
            sim.cycle = 1;
            sim.pc += 2;
        } break;
        
    }
}
void
op_lsr_zpx(Sim6502 &sim)
{
    switch (sim.cycle) {
        case 2: {
            sim.tmp8 = mem_read(sim, sim.pc + 1);
        } break;
        case 3: {
             /* discard memread BAL */
        } break;
        case 4: {
            sim.tmp16 = (sim.x + sim.tmp8) & 0xFF;
        } break;
        case 5: {
            sim.tmp8 = mem_read(sim, sim.tmp16);
        } break;
        case 6: {
             /* discard unmodified write */
        } break;
        case 7: {
            sim.pflags.carry = (sim.tmp8 & 0x01) != 0;
            sim.tmp8 = sim.tmp8 >> 1;
            setnz(sim, sim.tmp8);
            mem_write(sim, sim.tmp16, sim.tmp8);
            sim.cycle = 1;
            sim.pc += 2;
        } break;
        
    }
}
void
op_cli_imp(Sim6502 &sim)
{
    switch (sim.cycle) {
        case 2: {
            sim.pflags.intdis = 0;
            sim.cycle = 1;
            sim.pc += 1;
        } break;
        
    }
}
void
op_eor_absy(Sim6502 &sim)
{
    switch (sim.cycle) {
        case 2: {
            WRITE_LO(sim.tmp16, mem_read(sim, sim.pc + 1));
        } break;
        case 3: {
            WRITE_HI(sim.tmp16, mem_read(sim, sim.pc + 2));
        } break;
        case 4: {
            sim.tmp8 = mem_read(sim, sim.tmp16 + ((uint16)sim.y));
        } break;
        case 5: {
            sim.acc = sim.acc ^ sim.tmp8;
            setnz(sim, sim.acc);
            sim.cycle = 1;
            sim.pc += 3;
        } break;
        
    }
}
void
op_eor_absx(Sim6502 &sim)
{
    switch (sim.cycle) {
        case 2: {
            WRITE_LO(sim.tmp16, mem_read(sim, sim.pc + 1));
        } break;
        case 3: {
            WRITE_HI(sim.tmp16, mem_read(sim, sim.pc + 2));
        } break;
        case 4: {
            sim.tmp8 = mem_read(sim, sim.tmp16 + ((uint16)sim.x));
        } break;
        case 5: {
            sim.acc = sim.acc ^ sim.tmp8;
            setnz(sim, sim.acc);
            sim.cycle = 1;
            sim.pc += 3;
        } break;
        
    }
}
void
op_lsr_absx(Sim6502 &sim)
{
    switch (sim.cycle) {
        case 2: {
            WRITE_LO(sim.tmp16, mem_read(sim, sim.pc + 1));
        } break;
        case 3: {
            WRITE_HI(sim.tmp16, mem_read(sim, sim.pc + 2));
        } break;
        case 4: {
             /* discard read */
        } break;
        case 5: {
            sim.tmp8 = mem_read(sim, sim.tmp16 + ((uint16)sim.x));
        } break;
        case 6: {
             /* discard unmodified write */
        } break;
        case 7: {
            sim.pflags.carry = (sim.tmp8 & 0x01) != 0;
            sim.tmp8 = sim.tmp8 >> 1;
            setnz(sim, sim.tmp8);
            mem_write(sim, sim.tmp16 + ((uint16)sim.x), sim.tmp8);
            sim.cycle = 1;
            sim.pc += 3;
        } break;
        
    }
}
void
op_rts_imp(Sim6502 &sim)
{
    switch (sim.cycle) {
        case 2: {
             /* discard read PC + 1 */
        } break;
        case 3: {
             /* discard read SP */
        } break;
        case 4: {
            WRITE_LO(sim.pc, pop_stack(sim));
        } break;
        case 5: {
            WRITE_HI(sim.pc, pop_stack(sim));
        } break;
        case 6: {
             /* discard read PC */
            sim.cycle = 1;
            sim.pc += 1;
        } break;
        
    }
}
void
op_adc_izx(Sim6502 &sim)
{
    switch (sim.cycle) {
        case 2: {
            sim.tmp16 = mem_read(sim, sim.pc + 1);
        } break;
        case 3: {
            sim.tmp16 = (sim.tmp16 + sim.x) & 0xFF;
        } break;
        case 4: {
            sim.tmp8 = mem_read(sim, sim.tmp16 + 0);
        } break;
        case 5: {
            WRITE_HI(sim.tmp16, mem_read(sim, sim.tmp16 + 1));
        } break;
        case 6: {
            WRITE_LO(sim.tmp16, sim.tmp8);
        } break;
        case 7: {
            sim.tmp8 = mem_read(sim, sim.tmp16);
            sim.tmp16 = 0;
            sim.tmp16 += ((uint16)sim.acc);
            sim.tmp16 += ((uint16)sim.tmp8);
            sim.tmp16 += ((uint16)sim.pflags.carry);

            if (sim.pflags.deci) 
            { 
                sim.tmp16 = (sim.acc & 0x0F) + (sim.tmp8 & 0x0F) + sim.pflags.carry;
        
                if (sim.tmp16 > 0x09) {
                    sim.tmp16 += 0x06;
                }

                sim.tmp16 += (sim.acc & 0xF0) + (sim.tmp8 & 0xF0);
                if (sim.tmp16 > 0x99) {
                    sim.tmp16 += 0x60;
                    sim.pflags.carry = 1;
                } else {
                    sim.pflags.carry = 0;
                }
            } // BCD operation

            sim.pflags.ovrf = ((sim.acc & 0x80) == (sim.tmp8 & 0x80)) && 
                                          ((sim.acc & 0x80) != (sim.tmp16 & 0x80)) /* same sign operands, different sign result */;
            sim.pflags.carry = sim.tmp16 > 0xFF;
            sim.acc = READ_LO(sim.tmp16);
            setnz(sim, sim.acc);
            sim.cycle = 1;
            sim.pc += 2;
        } break;
        
    }
}
void
op_adc_zp(Sim6502 &sim)
{
    switch (sim.cycle) {
        case 2: {
            sim.tmp16 = ((uint16)mem_read(sim, sim.pc + 1));
        } break;
        case 3: {
            sim.tmp8 = mem_read(sim, sim.tmp16);
        } break;
        case 4: {
            sim.tmp16 = 0;
            sim.tmp16 += ((uint16)sim.acc);
            sim.tmp16 += ((uint16)sim.tmp8);
            sim.tmp16 += ((uint16)sim.pflags.carry);

            if (sim.pflags.deci) 
            { 
                sim.tmp16 = (sim.acc & 0x0F) + (sim.tmp8 & 0x0F) + sim.pflags.carry;
        
                if (sim.tmp16 > 0x09) {
                    sim.tmp16 += 0x06;
                }

                sim.tmp16 += (sim.acc & 0xF0) + (sim.tmp8 & 0xF0);
                if (sim.tmp16 > 0x99) {
                    sim.tmp16 += 0x60;
                    sim.pflags.carry = 1;
                } else {
                    sim.pflags.carry = 0;
                }
            } // BCD operation

            sim.pflags.ovrf = ((sim.acc & 0x80) == (sim.tmp8 & 0x80)) && 
                                          ((sim.acc & 0x80) != (sim.tmp16 & 0x80)) /* same sign operands, different sign result */;
            sim.pflags.carry = sim.tmp16 > 0xFF;

            sim.acc = READ_LO(sim.tmp16);
            setnz(sim, sim.acc);
            sim.cycle = 1;
            sim.pc += 2;
        } break;
        
    }
}
void
op_ror_zp(Sim6502 &sim)
{
    switch (sim.cycle) {
        case 2: {
            sim.tmp16 = ((uint16)mem_read(sim, sim.pc + 1));
        } break;
        case 3: {
            sim.tmp8 = mem_read(sim, sim.tmp16);
        } break;
        case 4: {
             /* discard read memwrite ADL */
        } break;
        case 5: {
            uint8 bit0 = (sim.tmp8 & 0x01) != 0;
            sim.tmp8 = ( sim.tmp8 >> 1) | ((uint8)sim.pflags.carry) << 7;
            sim.pflags.carry = bit0;
            setnz(sim, sim.tmp8);
            mem_write(sim, sim.tmp16, sim.tmp8);
            sim.cycle = 1;
            sim.pc += 2;
        } break;
        
    }
}
void
op_pla_imp(Sim6502 &sim)
{
    switch (sim.cycle) {
        case 2: {
             /* discard read PC + 1 */
        } break;
        case 3: {
             /* discard read SP */
        } break;
        case 4: {
            sim.acc = pop_stack(sim);
            setnz(sim, sim.acc);
            sim.cycle = 1;
            sim.pc += 1;
        } break;
        
    }
}
void
op_adc_imm(Sim6502 &sim)
{
    switch (sim.cycle) {
        case 2: {
            sim.tmp8 = mem_read(sim, sim.pc + 1);
        } break;
        case 3: {
            sim.tmp16 = 0;
            sim.tmp16 += ((uint16)sim.acc);
            sim.tmp16 += ((uint16)sim.tmp8);
            sim.tmp16 += ((uint16)sim.pflags.carry);

            if (sim.pflags.deci) 
            { 
                sim.tmp16 = (sim.acc & 0x0F) + (sim.tmp8 & 0x0F) + sim.pflags.carry;
        
                if (sim.tmp16 > 0x09) {
                    sim.tmp16 += 0x06;
                }

                sim.tmp16 += (sim.acc & 0xF0) + (sim.tmp8 & 0xF0);
                if (sim.tmp16 > 0x99) {
                    sim.tmp16 += 0x60;
                    sim.pflags.carry = 1;
                } else {
                    sim.pflags.carry = 0;
                }
            } // BCD operation

            sim.pflags.ovrf = ((sim.acc & 0x80) == (sim.tmp8 & 0x80)) && 
                                          ((sim.acc & 0x80) != (sim.tmp16 & 0x80)) /* same sign operands, different sign result */;
            sim.pflags.carry = sim.tmp16 > 0xFF;
            sim.acc = READ_LO(sim.tmp16);
            setnz(sim, sim.acc);
            sim.cycle = 1;
            sim.pc += 2;
        } break;
        
    }
}
void
op_ror_acc(Sim6502 &sim)
{
    switch (sim.cycle) {
        case 2: {
            uint8 bit0 = (sim.acc & 0x01) != 0;
            sim.acc = ( sim.acc >> 1) | ((uint8)sim.pflags.carry) << 7;
            sim.pflags.carry = bit0;
            setnz(sim, sim.acc);
            sim.cycle = 1;
            sim.pc += 1;
        } break;
        
    }
}
void
op_jmp_idr(Sim6502 &sim)
{
    switch (sim.cycle) {
        case 2: {
            WRITE_LO(sim.tmp16, mem_read(sim, sim.pc + 1));
        } break;
        case 3: {
            WRITE_HI(sim.tmp16, mem_read(sim, sim.pc + 2));
        } break;
        case 4: {
            WRITE_LO(sim.pc, mem_read(sim, sim.tmp16 + 0));
        } break;
        case 5: {
            WRITE_HI(sim.pc, mem_read(sim, sim.tmp16 + 1));
            sim.cycle = 1;
        } break;
        
    }
}
void
op_adc_abs(Sim6502 &sim)
{
    switch (sim.cycle) {
        case 2: {
            WRITE_LO(sim.tmp16, mem_read(sim, sim.pc + 1));
        } break;
        case 3: {
            WRITE_HI(sim.tmp16, mem_read(sim, sim.pc + 2));
        } break;
        case 4: {
            sim.tmp8 = mem_read(sim, sim.tmp16 + ((uint16)0));
        } break;
        case 5: {
            sim.tmp16 = 0;
            sim.tmp16 += ((uint16)sim.acc);
            sim.tmp16 += ((uint16)sim.tmp8);
            sim.tmp16 += ((uint16)sim.pflags.carry);

            if (sim.pflags.deci) 
            { 
                sim.tmp16 = (sim.acc & 0x0F) + (sim.tmp8 & 0x0F) + sim.pflags.carry;
        
                if (sim.tmp16 > 0x09) {
                    sim.tmp16 += 0x06;
                }

                sim.tmp16 += (sim.acc & 0xF0) + (sim.tmp8 & 0xF0);
                if (sim.tmp16 > 0x99) {
                    sim.tmp16 += 0x60;
                    sim.pflags.carry = 1;
                } else {
                    sim.pflags.carry = 0;
                }
            } // BCD operation

            sim.pflags.ovrf = ((sim.acc & 0x80) == (sim.tmp8 & 0x80)) && 
                                          ((sim.acc & 0x80) != (sim.tmp16 & 0x80)) /* same sign operands, different sign result */;
            sim.pflags.carry = sim.tmp16 > 0xFF;
            sim.acc = READ_LO(sim.tmp16);
            setnz(sim, sim.acc);
            sim.cycle = 1;
            sim.pc += 3;
        } break;
        
    }
}
void
op_ror_abs(Sim6502 &sim)
{
    switch (sim.cycle) {
        case 2: {
            WRITE_LO(sim.tmp16, mem_read(sim, sim.pc + 1));
        } break;
        case 3: {
            WRITE_HI(sim.tmp16, mem_read(sim, sim.pc + 2));
        } break;
        case 4: {
            sim.tmp8 = mem_read(sim, sim.tmp16 + ((uint16)0));
        } break;
        case 5: {
             /* discard unmodified write */
        } break;
        case 6: {
            uint8 bit0 = (sim.tmp8 & 0x01) != 0;
            sim.tmp8 = ( sim.tmp8 >> 1) | ((uint8)sim.pflags.carry) << 7;
            sim.pflags.carry = bit0;
            setnz(sim, sim.tmp8);
            mem_write(sim, sim.tmp16 + ((uint16)0), sim.tmp8);
            sim.cycle = 1;
            sim.pc += 3;
        } break;
        
    }
}
void
op_bvs_rel(Sim6502 &sim)
{
    switch (sim.cycle) {
        case 2: {
            // test the opposite of the operation
            if ( !(sim.pflags.ovrf == 1) ) { sim.cycle = 1; sim.pc += 2; }
            
            // break on test methods that sucessfully branch to itself
            else if (mem_read(sim, sim.pc + 1) == 0xFE) { init_gui_debugging(sim); sim.exec_mode = ExecMode_Pause; }
        } break;
        case 3: {
            sim.tmp16 = mem_read(sim, sim.pc + 1);
            if (sim.tmp16 & 0x80) sim.tmp16 |= 0xFF00; // sign extend
            sim.tmp16 += sim.pc + 2;
            bool overflow = READ_HI(sim.pc + 2) != READ_HI(sim.tmp16);
            sim.pc = sim.tmp16;
            if (!overflow) { sim.cycle = 1; } // page boundary cross check
        } break;
        case 4: {
             /* discard increment high byte of T16 */
            sim.cycle = 1;
        } break;
        
    }
}
void
op_adc_izy(Sim6502 &sim)
{
    switch (sim.cycle) {
        case 2: {
            sim.tmp16 = mem_read(sim, sim.pc + 1);
        } break;
        case 3: {
            sim.tmp8 = mem_read(sim, sim.tmp16 + 0);
        } break;
        case 4: {
            WRITE_HI(sim.tmp16, mem_read(sim, sim.tmp16 + 1));
        } break;
        case 5: {
            WRITE_LO(sim.tmp16, sim.tmp8);
        } break;
        case 6: {
            sim.tmp16 += sim.y;
        } break;
        case 7: {
            sim.tmp8 = mem_read(sim, sim.tmp16);
            sim.tmp16 = 0;
            sim.tmp16 += ((uint16)sim.acc);
            sim.tmp16 += ((uint16)sim.tmp8);
            sim.tmp16 += ((uint16)sim.pflags.carry);

            if (sim.pflags.deci) 
            { 
                sim.tmp16 = (sim.acc & 0x0F) + (sim.tmp8 & 0x0F) + sim.pflags.carry;
        
                if (sim.tmp16 > 0x09) {
                    sim.tmp16 += 0x06;
                }

                sim.tmp16 += (sim.acc & 0xF0) + (sim.tmp8 & 0xF0);
                if (sim.tmp16 > 0x99) {
                    sim.tmp16 += 0x60;
                    sim.pflags.carry = 1;
                } else {
                    sim.pflags.carry = 0;
                }
            } // BCD operation

            sim.pflags.ovrf = ((sim.acc & 0x80) == (sim.tmp8 & 0x80)) && 
                                          ((sim.acc & 0x80) != (sim.tmp16 & 0x80)) /* same sign operands, different sign result */;
            sim.pflags.carry = sim.tmp16 > 0xFF;
            sim.acc = READ_LO(sim.tmp16);
            setnz(sim, sim.acc);
            sim.cycle = 1;
            sim.pc += 2;
        } break;
        
    }
}
void
op_adc_zpx(Sim6502 &sim)
{
    switch (sim.cycle) {
        case 2: {
            sim.tmp8 = mem_read(sim, sim.pc + 1);
        } break;
        case 3: {
             /* discard memread BAL */
        } break;
        case 4: {
            sim.tmp16 = (sim.x + sim.tmp8) & 0xFF;
        } break;
        case 5: {
            sim.tmp8 = mem_read(sim, sim.tmp16);
        } break;
        case 6: {
            sim.tmp16 = 0;
            sim.tmp16 += ((uint16)sim.acc);
            sim.tmp16 += ((uint16)sim.tmp8);
            sim.tmp16 += ((uint16)sim.pflags.carry);

            if (sim.pflags.deci) 
            { 
                sim.tmp16 = (sim.acc & 0x0F) + (sim.tmp8 & 0x0F) + sim.pflags.carry;
        
                if (sim.tmp16 > 0x09) {
                    sim.tmp16 += 0x06;
                }

                sim.tmp16 += (sim.acc & 0xF0) + (sim.tmp8 & 0xF0);
                if (sim.tmp16 > 0x99) {
                    sim.tmp16 += 0x60;
                    sim.pflags.carry = 1;
                } else {
                    sim.pflags.carry = 0;
                }
            } // BCD operation

            sim.pflags.ovrf = ((sim.acc & 0x80) == (sim.tmp8 & 0x80)) && 
                                          ((sim.acc & 0x80) != (sim.tmp16 & 0x80)) /* same sign operands, different sign result */;
            sim.pflags.carry = sim.tmp16 > 0xFF;
            sim.acc = READ_LO(sim.tmp16);
            setnz(sim, sim.acc);
            sim.cycle = 1;
            sim.pc += 2;
        } break;
        
    }
}
void
op_ror_zpx(Sim6502 &sim)
{
    switch (sim.cycle) {
        case 2: {
            sim.tmp8 = mem_read(sim, sim.pc + 1);
        } break;
        case 3: {
             /* discard memread BAL */
        } break;
        case 4: {
            sim.tmp16 = (sim.x + sim.tmp8) & 0xFF;
        } break;
        case 5: {
            sim.tmp8 = mem_read(sim, sim.tmp16);
        } break;
        case 6: {
             /* discard unmodified write */
        } break;
        case 7: {
            uint8 bit0 = (sim.tmp8 & 0x01) != 0;
            sim.tmp8 = ( sim.tmp8 >> 1) | ((uint8)sim.pflags.carry) << 7;
            sim.pflags.carry = bit0;
            setnz(sim, sim.tmp8);
            mem_write(sim, sim.tmp16, sim.tmp8);
            sim.cycle = 1;
            sim.pc += 2;
        } break;
        
    }
}
void
op_sei_imp(Sim6502 &sim)
{
    switch (sim.cycle) {
        case 2: {
            sim.pflags.intdis = 1;
            sim.cycle = 1;
            sim.pc += 1;
        } break;
        
    }
}
void
op_adc_absy(Sim6502 &sim)
{
    switch (sim.cycle) {
        case 2: {
            WRITE_LO(sim.tmp16, mem_read(sim, sim.pc + 1));
        } break;
        case 3: {
            WRITE_HI(sim.tmp16, mem_read(sim, sim.pc + 2));
        } break;
        case 4: {
            sim.tmp8 = mem_read(sim, sim.tmp16 + ((uint16)sim.y));
        } break;
        case 5: {
            sim.tmp16 = 0;
            sim.tmp16 += ((uint16)sim.acc);
            sim.tmp16 += ((uint16)sim.tmp8);
            sim.tmp16 += ((uint16)sim.pflags.carry);

            if (sim.pflags.deci) 
            { 
                sim.tmp16 = (sim.acc & 0x0F) + (sim.tmp8 & 0x0F) + sim.pflags.carry;
        
                if (sim.tmp16 > 0x09) {
                    sim.tmp16 += 0x06;
                }

                sim.tmp16 += (sim.acc & 0xF0) + (sim.tmp8 & 0xF0);
                if (sim.tmp16 > 0x99) {
                    sim.tmp16 += 0x60;
                    sim.pflags.carry = 1;
                } else {
                    sim.pflags.carry = 0;
                }
            } // BCD operation

            sim.pflags.ovrf = ((sim.acc & 0x80) == (sim.tmp8 & 0x80)) && 
                                          ((sim.acc & 0x80) != (sim.tmp16 & 0x80)) /* same sign operands, different sign result */;
            sim.pflags.carry = sim.tmp16 > 0xFF;
            sim.acc = READ_LO(sim.tmp16);
            setnz(sim, sim.acc);
            sim.cycle = 1;
            sim.pc += 3;
        } break;
        
    }
}
void
op_adc_absx(Sim6502 &sim)
{
    switch (sim.cycle) {
        case 2: {
            WRITE_LO(sim.tmp16, mem_read(sim, sim.pc + 1));
        } break;
        case 3: {
            WRITE_HI(sim.tmp16, mem_read(sim, sim.pc + 2));
        } break;
        case 4: {
            sim.tmp8 = mem_read(sim, sim.tmp16 + ((uint16)sim.x));
        } break;
        case 5: {
            sim.tmp16 = 0;
            sim.tmp16 += ((uint16)sim.acc);
            sim.tmp16 += ((uint16)sim.tmp8);
            sim.tmp16 += ((uint16)sim.pflags.carry);

            if (sim.pflags.deci) 
            { 
                sim.tmp16 = (sim.acc & 0x0F) + (sim.tmp8 & 0x0F) + sim.pflags.carry;
        
                if (sim.tmp16 > 0x09) {
                    sim.tmp16 += 0x06;
                }

                sim.tmp16 += (sim.acc & 0xF0) + (sim.tmp8 & 0xF0);
                if (sim.tmp16 > 0x99) {
                    sim.tmp16 += 0x60;
                    sim.pflags.carry = 1;
                } else {
                    sim.pflags.carry = 0;
                }
            } // BCD operation

            sim.pflags.ovrf = ((sim.acc & 0x80) == (sim.tmp8 & 0x80)) && 
                                          ((sim.acc & 0x80) != (sim.tmp16 & 0x80)) /* same sign operands, different sign result */;
            sim.pflags.carry = sim.tmp16 > 0xFF;
            sim.acc = READ_LO(sim.tmp16);
            setnz(sim, sim.acc);
            sim.cycle = 1;
            sim.pc += 3;
        } break;
        
    }
}
void
op_ror_absx(Sim6502 &sim)
{
    switch (sim.cycle) {
        case 2: {
            WRITE_LO(sim.tmp16, mem_read(sim, sim.pc + 1));
        } break;
        case 3: {
            WRITE_HI(sim.tmp16, mem_read(sim, sim.pc + 2));
        } break;
        case 4: {
             /* discard read */
        } break;
        case 5: {
            sim.tmp8 = mem_read(sim, sim.tmp16 + ((uint16)sim.x));
        } break;
        case 6: {
             /* discard unmodified write */
        } break;
        case 7: {
            uint8 bit0 = (sim.tmp8 & 0x01) != 0;
            sim.tmp8 = ( sim.tmp8 >> 1) | ((uint8)sim.pflags.carry) << 7;
            sim.pflags.carry = bit0;
            setnz(sim, sim.tmp8);
            mem_write(sim, sim.tmp16 + ((uint16)sim.x), sim.tmp8);
            sim.cycle = 1;
            sim.pc += 3;
        } break;
        
    }
}
void
op_sta_izx(Sim6502 &sim)
{
    switch (sim.cycle) {
        case 2: {
            sim.tmp16 = mem_read(sim, sim.pc + 1);
        } break;
        case 3: {
            sim.tmp16 = (sim.tmp16 + sim.x) & 0xFF;
        } break;
        case 4: {
            sim.tmp8 = mem_read(sim, sim.tmp16 + 0);
        } break;
        case 5: {
            WRITE_HI(sim.tmp16, mem_read(sim, sim.tmp16 + 1));
        } break;
        case 6: {
            WRITE_LO(sim.tmp16, sim.tmp8);
        } break;
        case 7: {
            sim.tmp8 = mem_read(sim, sim.tmp16);
            sim.tmp8 = sim.acc;
            mem_write(sim, sim.tmp16, sim.tmp8);
            sim.cycle = 1;
            sim.pc += 2;
        } break;
        
    }
}
void
op_sty_zp(Sim6502 &sim)
{
    switch (sim.cycle) {
        case 2: {
            sim.tmp16 = ((uint16)mem_read(sim, sim.pc + 1));
        } break;
        case 3: {
            sim.tmp8 = sim.y;
            mem_write(sim, sim.tmp16, sim.tmp8);
            sim.cycle = 1;
            sim.pc += 2;
        } break;
        
    }
}
void
op_sta_zp(Sim6502 &sim)
{
    switch (sim.cycle) {
        case 2: {
            sim.tmp16 = ((uint16)mem_read(sim, sim.pc + 1));
        } break;
        case 3: {
            sim.tmp8 = sim.acc;
            mem_write(sim, sim.tmp16, sim.tmp8);
            sim.cycle = 1;
            sim.pc += 2;
        } break;
        
    }
}
void
op_stx_zp(Sim6502 &sim)
{
    switch (sim.cycle) {
        case 2: {
            sim.tmp16 = ((uint16)mem_read(sim, sim.pc + 1));
        } break;
        case 3: {
            sim.tmp8 = sim.x;
            mem_write(sim, sim.tmp16, sim.tmp8);
            sim.cycle = 1;
            sim.pc += 2;
        } break;
        
    }
}
void
op_dey_imp(Sim6502 &sim)
{
    switch (sim.cycle) {
        case 2: {
            sim.y -= 1;
            setnz(sim, sim.y);
            sim.cycle = 1;
            sim.pc += 1;
        } break;
        
    }
}
void
op_txa_imp(Sim6502 &sim)
{
    switch (sim.cycle) {
        case 2: {
            sim.acc = sim.x;
            setnz(sim, sim.acc);
            sim.cycle = 1;
            sim.pc += 1;
        } break;
        
    }
}
void
op_sty_abs(Sim6502 &sim)
{
    switch (sim.cycle) {
        case 2: {
            WRITE_LO(sim.tmp16, mem_read(sim, sim.pc + 1));
        } break;
        case 3: {
            WRITE_HI(sim.tmp16, mem_read(sim, sim.pc + 2));
        } break;
        case 4: {
            sim.tmp8 = sim.y;
            mem_write(sim, sim.tmp16 + ((uint16)0), sim.tmp8);
            sim.cycle = 1;
            sim.pc += 3;
        } break;
        
    }
}
void
op_sta_abs(Sim6502 &sim)
{
    switch (sim.cycle) {
        case 2: {
            WRITE_LO(sim.tmp16, mem_read(sim, sim.pc + 1));
        } break;
        case 3: {
            WRITE_HI(sim.tmp16, mem_read(sim, sim.pc + 2));
        } break;
        case 4: {
            sim.tmp8 = sim.acc;
            mem_write(sim, sim.tmp16 + ((uint16)0), sim.tmp8);
            sim.cycle = 1;
            sim.pc += 3;
        } break;
        
    }
}
void
op_stx_abs(Sim6502 &sim)
{
    switch (sim.cycle) {
        case 2: {
            WRITE_LO(sim.tmp16, mem_read(sim, sim.pc + 1));
        } break;
        case 3: {
            WRITE_HI(sim.tmp16, mem_read(sim, sim.pc + 2));
        } break;
        case 4: {
            sim.tmp8 = sim.x;
            mem_write(sim, sim.tmp16 + ((uint16)0), sim.tmp8);
            sim.cycle = 1;
            sim.pc += 3;
        } break;
        
    }
}
void
op_bcc_rel(Sim6502 &sim)
{
    switch (sim.cycle) {
        case 2: {
            // test the opposite of the operation
            if ( !(sim.pflags.carry == 0) ) { sim.cycle = 1; sim.pc += 2; }
            
            // break on test methods that sucessfully branch to itself
            else if (mem_read(sim, sim.pc + 1) == 0xFE) { init_gui_debugging(sim); sim.exec_mode = ExecMode_Pause; }
        } break;
        case 3: {
            sim.tmp16 = mem_read(sim, sim.pc + 1);
            if (sim.tmp16 & 0x80) sim.tmp16 |= 0xFF00; // sign extend
            sim.tmp16 += sim.pc + 2;
            bool overflow = READ_HI(sim.pc + 2) != READ_HI(sim.tmp16);
            sim.pc = sim.tmp16;
            if (!overflow) { sim.cycle = 1; } // page boundary cross check
        } break;
        case 4: {
             /* discard increment high byte of T16 */
            sim.cycle = 1;
        } break;
        
    }
}
void
op_sta_izy(Sim6502 &sim)
{
    switch (sim.cycle) {
        case 2: {
            sim.tmp16 = mem_read(sim, sim.pc + 1);
        } break;
        case 3: {
            sim.tmp8 = mem_read(sim, sim.tmp16 + 0);
        } break;
        case 4: {
            WRITE_HI(sim.tmp16, mem_read(sim, sim.tmp16 + 1));
        } break;
        case 5: {
            WRITE_LO(sim.tmp16, sim.tmp8);
        } break;
        case 6: {
            sim.tmp16 += sim.y;
        } break;
        case 7: {
            sim.tmp8 = mem_read(sim, sim.tmp16);
            sim.tmp8 = sim.acc;
            mem_write(sim, sim.tmp16, sim.tmp8);
            sim.cycle = 1;
            sim.pc += 2;
        } break;
        
    }
}
void
op_sty_zpx(Sim6502 &sim)
{
    switch (sim.cycle) {
        case 2: {
            sim.tmp8 = mem_read(sim, sim.pc + 1);
        } break;
        case 3: {
             /* discard memread BAL */
        } break;
        case 4: {
            sim.tmp16 = (sim.x + sim.tmp8) & 0xFF;
        } break;
        case 5: {
            sim.tmp8 = mem_read(sim, sim.tmp16);
        } break;
        case 6: {
             /* discard unmodified write */
        } break;
        case 7: {
            sim.tmp8 = sim.y;
            mem_write(sim, sim.tmp16, sim.tmp8);
            sim.cycle = 1;
            sim.pc += 2;
        } break;
        
    }
}
void
op_sta_zpx(Sim6502 &sim)
{
    switch (sim.cycle) {
        case 2: {
            sim.tmp8 = mem_read(sim, sim.pc + 1);
        } break;
        case 3: {
             /* discard memread BAL */
        } break;
        case 4: {
            sim.tmp16 = (sim.x + sim.tmp8) & 0xFF;
        } break;
        case 5: {
            sim.tmp8 = mem_read(sim, sim.tmp16);
        } break;
        case 6: {
             /* discard unmodified write */
        } break;
        case 7: {
            sim.tmp8 = sim.acc;
            mem_write(sim, sim.tmp16, sim.tmp8);
            sim.cycle = 1;
            sim.pc += 2;
        } break;
        
    }
}
void
op_stx_zpy(Sim6502 &sim)
{
    switch (sim.cycle) {
        case 2: {
            sim.tmp8 = mem_read(sim, sim.pc + 1);
        } break;
        case 3: {
             /* discard memread BAL */
        } break;
        case 4: {
            sim.tmp16 = (sim.y + sim.tmp8) & 0xFF;
        } break;
        case 5: {
            sim.tmp8 = mem_read(sim, sim.tmp16);
        } break;
        case 6: {
             /* discard unmodified write */
        } break;
        case 7: {
            sim.tmp8 = sim.x;
            mem_write(sim, sim.tmp16, sim.tmp8);
            sim.cycle = 1;
            sim.pc += 2;
        } break;
        
    }
}
void
op_tya_imp(Sim6502 &sim)
{
    switch (sim.cycle) {
        case 2: {
            sim.acc = sim.y;
            setnz(sim, sim.acc);
            sim.cycle = 1;
            sim.pc += 1;
        } break;
        
    }
}
void
op_sta_absy(Sim6502 &sim)
{
    switch (sim.cycle) {
        case 2: {
            WRITE_LO(sim.tmp16, mem_read(sim, sim.pc + 1));
        } break;
        case 3: {
            WRITE_HI(sim.tmp16, mem_read(sim, sim.pc + 2));
        } break;
        case 4: {
             /* discard read */
        } break;
        case 5: {
            sim.tmp8 = mem_read(sim, sim.tmp16 + ((uint16)sim.y));
        } break;
        case 6: {
             /* discard unmodified write */
        } break;
        case 7: {
            sim.tmp8 = sim.acc;
            mem_write(sim, sim.tmp16 + ((uint16)sim.y), sim.tmp8);
            sim.cycle = 1;
            sim.pc += 3;
        } break;
        
    }
}
void
op_txs_imp(Sim6502 &sim)
{
    switch (sim.cycle) {
        case 2: {
            sim.sp = sim.x;
            sim.cycle = 1;
            sim.pc += 1;
        } break;
        
    }
}
void
op_sta_absx(Sim6502 &sim)
{
    switch (sim.cycle) {
        case 2: {
            WRITE_LO(sim.tmp16, mem_read(sim, sim.pc + 1));
        } break;
        case 3: {
            WRITE_HI(sim.tmp16, mem_read(sim, sim.pc + 2));
        } break;
        case 4: {
             /* discard read */
        } break;
        case 5: {
            sim.tmp8 = mem_read(sim, sim.tmp16 + ((uint16)sim.x));
        } break;
        case 6: {
             /* discard unmodified write */
        } break;
        case 7: {
            sim.tmp8 = sim.acc;
            mem_write(sim, sim.tmp16 + ((uint16)sim.x), sim.tmp8);
            sim.cycle = 1;
            sim.pc += 3;
        } break;
        
    }
}
void
op_ldy_imm(Sim6502 &sim)
{
    switch (sim.cycle) {
        case 2: {
            sim.tmp8 = mem_read(sim, sim.pc + 1);
        } break;
        case 3: {
            sim.y = sim.tmp8;
            setnz(sim, sim.y);
            sim.cycle = 1;
            sim.pc += 2;
        } break;
        
    }
}
void
op_lda_izx(Sim6502 &sim)
{
    switch (sim.cycle) {
        case 2: {
            sim.tmp16 = mem_read(sim, sim.pc + 1);
        } break;
        case 3: {
            sim.tmp16 = (sim.tmp16 + sim.x) & 0xFF;
        } break;
        case 4: {
            sim.tmp8 = mem_read(sim, sim.tmp16 + 0);
        } break;
        case 5: {
            WRITE_HI(sim.tmp16, mem_read(sim, sim.tmp16 + 1));
        } break;
        case 6: {
            WRITE_LO(sim.tmp16, sim.tmp8);
        } break;
        case 7: {
            sim.tmp8 = mem_read(sim, sim.tmp16);
            sim.acc = sim.tmp8;
            setnz(sim, sim.acc);
            sim.cycle = 1;
            sim.pc += 2;
        } break;
        
    }
}
void
op_ldx_imm(Sim6502 &sim)
{
    switch (sim.cycle) {
        case 2: {
            sim.tmp8 = mem_read(sim, sim.pc + 1);
        } break;
        case 3: {
            sim.x = sim.tmp8;
            setnz(sim, sim.x);
            sim.cycle = 1;
            sim.pc += 2;
        } break;
        
    }
}
void
op_ldy_zp(Sim6502 &sim)
{
    switch (sim.cycle) {
        case 2: {
            sim.tmp16 = ((uint16)mem_read(sim, sim.pc + 1));
        } break;
        case 3: {
            sim.tmp8 = mem_read(sim, sim.tmp16);
        } break;
        case 4: {
            sim.y = sim.tmp8;
            setnz(sim, sim.y);
            sim.cycle = 1;
            sim.pc += 2;
        } break;
        
    }
}
void
op_lda_zp(Sim6502 &sim)
{
    switch (sim.cycle) {
        case 2: {
            sim.tmp16 = ((uint16)mem_read(sim, sim.pc + 1));
        } break;
        case 3: {
            sim.tmp8 = mem_read(sim, sim.tmp16);
        } break;
        case 4: {
            sim.acc = sim.tmp8;
            setnz(sim, sim.acc);
            sim.cycle = 1;
            sim.pc += 2;
        } break;
        
    }
}
void
op_ldx_zp(Sim6502 &sim)
{
    switch (sim.cycle) {
        case 2: {
            sim.tmp16 = ((uint16)mem_read(sim, sim.pc + 1));
        } break;
        case 3: {
            sim.tmp8 = mem_read(sim, sim.tmp16);
        } break;
        case 4: {
            sim.x = sim.tmp8;
            setnz(sim, sim.x);
            sim.cycle = 1;
            sim.pc += 2;
        } break;
        
    }
}
void
op_tay_imp(Sim6502 &sim)
{
    switch (sim.cycle) {
        case 2: {
            sim.y = sim.acc;
            setnz(sim, sim.y);
            sim.cycle = 1;
            sim.pc += 1;
        } break;
        
    }
}
void
op_lda_imm(Sim6502 &sim)
{
    switch (sim.cycle) {
        case 2: {
            sim.tmp8 = mem_read(sim, sim.pc + 1);
        } break;
        case 3: {
            sim.acc = sim.tmp8;
            setnz(sim, sim.acc);
            sim.cycle = 1;
            sim.pc += 2;
        } break;
        
    }
}
void
op_tax_imp(Sim6502 &sim)
{
    switch (sim.cycle) {
        case 2: {
            sim.x = sim.acc;
            setnz(sim, sim.x);
            sim.cycle = 1;
            sim.pc += 1;
        } break;
        
    }
}
void
op_ldy_abs(Sim6502 &sim)
{
    switch (sim.cycle) {
        case 2: {
            WRITE_LO(sim.tmp16, mem_read(sim, sim.pc + 1));
        } break;
        case 3: {
            WRITE_HI(sim.tmp16, mem_read(sim, sim.pc + 2));
        } break;
        case 4: {
            sim.tmp8 = mem_read(sim, sim.tmp16 + ((uint16)0));
        } break;
        case 5: {
            sim.y = sim.tmp8;
            setnz(sim, sim.y);
            sim.cycle = 1;
            sim.pc += 3;
        } break;
        
    }
}
void
op_lda_abs(Sim6502 &sim)
{
    switch (sim.cycle) {
        case 2: {
            WRITE_LO(sim.tmp16, mem_read(sim, sim.pc + 1));
        } break;
        case 3: {
            WRITE_HI(sim.tmp16, mem_read(sim, sim.pc + 2));
        } break;
        case 4: {
            sim.tmp8 = mem_read(sim, sim.tmp16 + ((uint16)0));
        } break;
        case 5: {
            sim.acc = sim.tmp8;
            setnz(sim, sim.acc);
            sim.cycle = 1;
            sim.pc += 3;
        } break;
        
    }
}
void
op_ldx_abs(Sim6502 &sim)
{
    switch (sim.cycle) {
        case 2: {
            WRITE_LO(sim.tmp16, mem_read(sim, sim.pc + 1));
        } break;
        case 3: {
            WRITE_HI(sim.tmp16, mem_read(sim, sim.pc + 2));
        } break;
        case 4: {
            sim.tmp8 = mem_read(sim, sim.tmp16 + ((uint16)0));
        } break;
        case 5: {
            sim.x = sim.tmp8;
            setnz(sim, sim.x);
            sim.cycle = 1;
            sim.pc += 3;
        } break;
        
    }
}
void
op_bcs_rel(Sim6502 &sim)
{
    switch (sim.cycle) {
        case 2: {
            // test the opposite of the operation
            if ( !(sim.pflags.carry == 1) ) { sim.cycle = 1; sim.pc += 2; }
            
            // break on test methods that sucessfully branch to itself
            else if (mem_read(sim, sim.pc + 1) == 0xFE) { init_gui_debugging(sim); sim.exec_mode = ExecMode_Pause; }
        } break;
        case 3: {
            sim.tmp16 = mem_read(sim, sim.pc + 1);
            if (sim.tmp16 & 0x80) sim.tmp16 |= 0xFF00; // sign extend
            sim.tmp16 += sim.pc + 2;
            bool overflow = READ_HI(sim.pc + 2) != READ_HI(sim.tmp16);
            sim.pc = sim.tmp16;
            if (!overflow) { sim.cycle = 1; } // page boundary cross check
        } break;
        case 4: {
             /* discard increment high byte of T16 */
            sim.cycle = 1;
        } break;
        
    }
}
void
op_lda_izy(Sim6502 &sim)
{
    switch (sim.cycle) {
        case 2: {
            sim.tmp16 = mem_read(sim, sim.pc + 1);
        } break;
        case 3: {
            sim.tmp8 = mem_read(sim, sim.tmp16 + 0);
        } break;
        case 4: {
            WRITE_HI(sim.tmp16, mem_read(sim, sim.tmp16 + 1));
        } break;
        case 5: {
            WRITE_LO(sim.tmp16, sim.tmp8);
        } break;
        case 6: {
            sim.tmp16 += sim.y;
        } break;
        case 7: {
            sim.tmp8 = mem_read(sim, sim.tmp16);
            sim.acc = sim.tmp8;
            setnz(sim, sim.acc);
            sim.cycle = 1;
            sim.pc += 2;
        } break;
        
    }
}
void
op_ldy_zpx(Sim6502 &sim)
{
    switch (sim.cycle) {
        case 2: {
            sim.tmp8 = mem_read(sim, sim.pc + 1);
        } break;
        case 3: {
             /* discard memread BAL */
        } break;
        case 4: {
            sim.tmp16 = (sim.x + sim.tmp8) & 0xFF;
        } break;
        case 5: {
            sim.tmp8 = mem_read(sim, sim.tmp16);
        } break;
        case 6: {
            sim.y = sim.tmp8;
            setnz(sim, sim.y);
            sim.cycle = 1;
            sim.pc += 2;
        } break;
        
    }
}
void
op_lda_zpx(Sim6502 &sim)
{
    switch (sim.cycle) {
        case 2: {
            sim.tmp8 = mem_read(sim, sim.pc + 1);
        } break;
        case 3: {
             /* discard memread BAL */
        } break;
        case 4: {
            sim.tmp16 = (sim.x + sim.tmp8) & 0xFF;
        } break;
        case 5: {
            sim.tmp8 = mem_read(sim, sim.tmp16);
        } break;
        case 6: {
            sim.acc = sim.tmp8;
            setnz(sim, sim.acc);
            sim.cycle = 1;
            sim.pc += 2;
        } break;
        
    }
}
void
op_ldx_zpy(Sim6502 &sim)
{
    switch (sim.cycle) {
        case 2: {
            sim.tmp8 = mem_read(sim, sim.pc + 1);
        } break;
        case 3: {
             /* discard memread BAL */
        } break;
        case 4: {
            sim.tmp16 = (sim.y + sim.tmp8) & 0xFF;
        } break;
        case 5: {
            sim.tmp8 = mem_read(sim, sim.tmp16);
        } break;
        case 6: {
            sim.x = sim.tmp8;
            setnz(sim, sim.x);
            sim.cycle = 1;
            sim.pc += 2;
        } break;
        
    }
}
void
op_clv_imp(Sim6502 &sim)
{
    switch (sim.cycle) {
        case 2: {
            sim.pflags.ovrf = 0;
            sim.cycle = 1;
            sim.pc += 1;
        } break;
        
    }
}
void
op_lda_absy(Sim6502 &sim)
{
    switch (sim.cycle) {
        case 2: {
            WRITE_LO(sim.tmp16, mem_read(sim, sim.pc + 1));
        } break;
        case 3: {
            WRITE_HI(sim.tmp16, mem_read(sim, sim.pc + 2));
        } break;
        case 4: {
            sim.tmp8 = mem_read(sim, sim.tmp16 + ((uint16)sim.y));
        } break;
        case 5: {
            sim.acc = sim.tmp8;
            setnz(sim, sim.acc);
            sim.cycle = 1;
            sim.pc += 3;
        } break;
        
    }
}
void
op_tsx_imp(Sim6502 &sim)
{
    switch (sim.cycle) {
        case 2: {
            sim.x = sim.sp;
            setnz(sim, sim.x);
            sim.cycle = 1;
            sim.pc += 1;
        } break;
        
    }
}
void
op_ldy_absx(Sim6502 &sim)
{
    switch (sim.cycle) {
        case 2: {
            WRITE_LO(sim.tmp16, mem_read(sim, sim.pc + 1));
        } break;
        case 3: {
            WRITE_HI(sim.tmp16, mem_read(sim, sim.pc + 2));
        } break;
        case 4: {
            sim.tmp8 = mem_read(sim, sim.tmp16 + ((uint16)sim.x));
        } break;
        case 5: {
            sim.y = sim.tmp8;
            setnz(sim, sim.y);
            sim.cycle = 1;
            sim.pc += 3;
        } break;
        
    }
}
void
op_lda_absx(Sim6502 &sim)
{
    switch (sim.cycle) {
        case 2: {
            WRITE_LO(sim.tmp16, mem_read(sim, sim.pc + 1));
        } break;
        case 3: {
            WRITE_HI(sim.tmp16, mem_read(sim, sim.pc + 2));
        } break;
        case 4: {
            sim.tmp8 = mem_read(sim, sim.tmp16 + ((uint16)sim.x));
        } break;
        case 5: {
            sim.acc = sim.tmp8;
            setnz(sim, sim.acc);
            sim.cycle = 1;
            sim.pc += 3;
        } break;
        
    }
}
void
op_ldx_absy(Sim6502 &sim)
{
    switch (sim.cycle) {
        case 2: {
            WRITE_LO(sim.tmp16, mem_read(sim, sim.pc + 1));
        } break;
        case 3: {
            WRITE_HI(sim.tmp16, mem_read(sim, sim.pc + 2));
        } break;
        case 4: {
            sim.tmp8 = mem_read(sim, sim.tmp16 + ((uint16)sim.y));
        } break;
        case 5: {
            sim.x = sim.tmp8;
            setnz(sim, sim.x);
            sim.cycle = 1;
            sim.pc += 3;
        } break;
        
    }
}
void
op_cpy_imm(Sim6502 &sim)
{
    switch (sim.cycle) {
        case 2: {
            sim.tmp8 = mem_read(sim, sim.pc + 1);
        } break;
        case 3: {
            sim.tmp8 = sim.y - sim.tmp8;
            setnz(sim, sim.tmp8);
            sim.pflags.carry = sim.y >= sim.tmp8;
            sim.cycle = 1;
            sim.pc += 2;
        } break;
        
    }
}
void
op_cmp_izx(Sim6502 &sim)
{
    switch (sim.cycle) {
        case 2: {
            sim.tmp16 = mem_read(sim, sim.pc + 1);
        } break;
        case 3: {
            sim.tmp16 = (sim.tmp16 + sim.x) & 0xFF;
        } break;
        case 4: {
            sim.tmp8 = mem_read(sim, sim.tmp16 + 0);
        } break;
        case 5: {
            WRITE_HI(sim.tmp16, mem_read(sim, sim.tmp16 + 1));
        } break;
        case 6: {
            WRITE_LO(sim.tmp16, sim.tmp8);
        } break;
        case 7: {
            sim.tmp8 = mem_read(sim, sim.tmp16);
            sim.tmp8 = sim.acc - sim.tmp8;
            setnz(sim, sim.tmp8);
            sim.pflags.carry = sim.acc >= sim.tmp8;
            sim.cycle = 1;
            sim.pc += 2;
        } break;
        
    }
}
void
op_cpy_zp(Sim6502 &sim)
{
    switch (sim.cycle) {
        case 2: {
            sim.tmp16 = ((uint16)mem_read(sim, sim.pc + 1));
        } break;
        case 3: {
            sim.tmp8 = mem_read(sim, sim.tmp16);
        } break;
        case 4: {
            sim.tmp8 = sim.y - sim.tmp8;
            setnz(sim, sim.tmp8);
            sim.pflags.carry = sim.y >= sim.tmp8;
            sim.cycle = 1;
            sim.pc += 2;
        } break;
        
    }
}
void
op_cmp_zp(Sim6502 &sim)
{
    switch (sim.cycle) {
        case 2: {
            sim.tmp16 = ((uint16)mem_read(sim, sim.pc + 1));
        } break;
        case 3: {
            sim.tmp8 = mem_read(sim, sim.tmp16);
        } break;
        case 4: {
            sim.tmp8 = sim.acc - sim.tmp8;
            setnz(sim, sim.tmp8);
            sim.pflags.carry = sim.acc >= sim.tmp8;
            sim.cycle = 1;
            sim.pc += 2;
        } break;
        
    }
}
void
op_dec_zp(Sim6502 &sim)
{
    switch (sim.cycle) {
        case 2: {
            sim.tmp16 = ((uint16)mem_read(sim, sim.pc + 1));
        } break;
        case 3: {
            sim.tmp8 = mem_read(sim, sim.tmp16);
        } break;
        case 4: {
             /* discard read memwrite ADL */
        } break;
        case 5: {
            sim.tmp8 -= 1;
            setnz(sim, sim.tmp8);
            mem_write(sim, sim.tmp16, sim.tmp8);
            sim.cycle = 1;
            sim.pc += 2;
        } break;
        
    }
}
void
op_iny_imp(Sim6502 &sim)
{
    switch (sim.cycle) {
        case 2: {
            sim.y += 1;
            setnz(sim, sim.y);
            sim.cycle = 1;
            sim.pc += 1;
        } break;
        
    }
}
void
op_cmp_imm(Sim6502 &sim)
{
    switch (sim.cycle) {
        case 2: {
            sim.tmp8 = mem_read(sim, sim.pc + 1);
        } break;
        case 3: {
            sim.tmp8 = sim.acc - sim.tmp8;
            setnz(sim, sim.tmp8);
            sim.pflags.carry = sim.acc >= sim.tmp8;
            sim.cycle = 1;
            sim.pc += 2;
        } break;
        
    }
}
void
op_dex_imp(Sim6502 &sim)
{
    switch (sim.cycle) {
        case 2: {
            sim.x -= 1;
            setnz(sim, sim.x);
            sim.cycle = 1;
            sim.pc += 1;
        } break;
        
    }
}
void
op_cpy_abs(Sim6502 &sim)
{
    switch (sim.cycle) {
        case 2: {
            WRITE_LO(sim.tmp16, mem_read(sim, sim.pc + 1));
        } break;
        case 3: {
            WRITE_HI(sim.tmp16, mem_read(sim, sim.pc + 2));
        } break;
        case 4: {
            sim.tmp8 = mem_read(sim, sim.tmp16 + ((uint16)0));
        } break;
        case 5: {
            sim.tmp8 = sim.y - sim.tmp8;
            setnz(sim, sim.tmp8);
            sim.pflags.carry = sim.y >= sim.tmp8;
            sim.cycle = 1;
            sim.pc += 3;
        } break;
        
    }
}
void
op_cmp_abs(Sim6502 &sim)
{
    switch (sim.cycle) {
        case 2: {
            WRITE_LO(sim.tmp16, mem_read(sim, sim.pc + 1));
        } break;
        case 3: {
            WRITE_HI(sim.tmp16, mem_read(sim, sim.pc + 2));
        } break;
        case 4: {
            sim.tmp8 = mem_read(sim, sim.tmp16 + ((uint16)0));
        } break;
        case 5: {
            sim.tmp8 = sim.acc - sim.tmp8;
            setnz(sim, sim.tmp8);
            sim.pflags.carry = sim.acc >= sim.tmp8;
            sim.cycle = 1;
            sim.pc += 3;
        } break;
        
    }
}
void
op_dec_abs(Sim6502 &sim)
{
    switch (sim.cycle) {
        case 2: {
            WRITE_LO(sim.tmp16, mem_read(sim, sim.pc + 1));
        } break;
        case 3: {
            WRITE_HI(sim.tmp16, mem_read(sim, sim.pc + 2));
        } break;
        case 4: {
            sim.tmp8 = mem_read(sim, sim.tmp16 + ((uint16)0));
        } break;
        case 5: {
             /* discard unmodified write */
        } break;
        case 6: {
            sim.tmp8 -= 1;
            setnz(sim, sim.tmp8);
            mem_write(sim, sim.tmp16 + ((uint16)0), sim.tmp8);
            sim.cycle = 1;
            sim.pc += 3;
        } break;
        
    }
}
void
op_bne_rel(Sim6502 &sim)
{
    switch (sim.cycle) {
        case 2: {
            // test the opposite of the operation
            if ( !(sim.pflags.zero == 0) ) { sim.cycle = 1; sim.pc += 2; }
            
            // break on test methods that sucessfully branch to itself
            else if (mem_read(sim, sim.pc + 1) == 0xFE) { init_gui_debugging(sim); sim.exec_mode = ExecMode_Pause; }
        } break;
        case 3: {
            sim.tmp16 = mem_read(sim, sim.pc + 1);
            if (sim.tmp16 & 0x80) sim.tmp16 |= 0xFF00; // sign extend
            sim.tmp16 += sim.pc + 2;
            bool overflow = READ_HI(sim.pc + 2) != READ_HI(sim.tmp16);
            sim.pc = sim.tmp16;
            if (!overflow) { sim.cycle = 1; } // page boundary cross check
        } break;
        case 4: {
             /* discard increment high byte of T16 */
            sim.cycle = 1;
        } break;
        
    }
}
void
op_cmp_izy(Sim6502 &sim)
{
    switch (sim.cycle) {
        case 2: {
            sim.tmp16 = mem_read(sim, sim.pc + 1);
        } break;
        case 3: {
            sim.tmp8 = mem_read(sim, sim.tmp16 + 0);
        } break;
        case 4: {
            WRITE_HI(sim.tmp16, mem_read(sim, sim.tmp16 + 1));
        } break;
        case 5: {
            WRITE_LO(sim.tmp16, sim.tmp8);
        } break;
        case 6: {
            sim.tmp16 += sim.y;
        } break;
        case 7: {
            sim.tmp8 = mem_read(sim, sim.tmp16);
            sim.tmp8 = sim.acc - sim.tmp8;
            setnz(sim, sim.tmp8);
            sim.pflags.carry = sim.acc >= sim.tmp8;
            sim.cycle = 1;
            sim.pc += 2;
        } break;
        
    }
}
void
op_cmp_zpx(Sim6502 &sim)
{
    switch (sim.cycle) {
        case 2: {
            sim.tmp8 = mem_read(sim, sim.pc + 1);
        } break;
        case 3: {
             /* discard memread BAL */
        } break;
        case 4: {
            sim.tmp16 = (sim.x + sim.tmp8) & 0xFF;
        } break;
        case 5: {
            sim.tmp8 = mem_read(sim, sim.tmp16);
        } break;
        case 6: {
            sim.tmp8 = sim.acc - sim.tmp8;
            setnz(sim, sim.tmp8);
            sim.pflags.carry = sim.acc >= sim.tmp8;
            sim.cycle = 1;
            sim.pc += 2;
        } break;
        
    }
}
void
op_dec_zpx(Sim6502 &sim)
{
    switch (sim.cycle) {
        case 2: {
            sim.tmp8 = mem_read(sim, sim.pc + 1);
        } break;
        case 3: {
             /* discard memread BAL */
        } break;
        case 4: {
            sim.tmp16 = (sim.x + sim.tmp8) & 0xFF;
        } break;
        case 5: {
            sim.tmp8 = mem_read(sim, sim.tmp16);
        } break;
        case 6: {
             /* discard unmodified write */
        } break;
        case 7: {
            sim.tmp8 -= 1;
            setnz(sim, sim.tmp8);
            mem_write(sim, sim.tmp16, sim.tmp8);
            sim.cycle = 1;
            sim.pc += 2;
        } break;
        
    }
}
void
op_cld_imp(Sim6502 &sim)
{
    switch (sim.cycle) {
        case 2: {
            sim.pflags.deci = 0;
            sim.cycle = 1;
            sim.pc += 1;
        } break;
        
    }
}
void
op_cmp_absy(Sim6502 &sim)
{
    switch (sim.cycle) {
        case 2: {
            WRITE_LO(sim.tmp16, mem_read(sim, sim.pc + 1));
        } break;
        case 3: {
            WRITE_HI(sim.tmp16, mem_read(sim, sim.pc + 2));
        } break;
        case 4: {
            sim.tmp8 = mem_read(sim, sim.tmp16 + ((uint16)sim.y));
        } break;
        case 5: {
            sim.tmp8 = sim.acc - sim.tmp8;
            setnz(sim, sim.tmp8);
            sim.pflags.carry = sim.acc >= sim.tmp8;
            sim.cycle = 1;
            sim.pc += 3;
        } break;
        
    }
}
void
op_cmp_absx(Sim6502 &sim)
{
    switch (sim.cycle) {
        case 2: {
            WRITE_LO(sim.tmp16, mem_read(sim, sim.pc + 1));
        } break;
        case 3: {
            WRITE_HI(sim.tmp16, mem_read(sim, sim.pc + 2));
        } break;
        case 4: {
            sim.tmp8 = mem_read(sim, sim.tmp16 + ((uint16)sim.x));
        } break;
        case 5: {
            sim.tmp8 = sim.acc - sim.tmp8;
            setnz(sim, sim.tmp8);
            sim.pflags.carry = sim.acc >= sim.tmp8;
            sim.cycle = 1;
            sim.pc += 3;
        } break;
        
    }
}
void
op_dec_absx(Sim6502 &sim)
{
    switch (sim.cycle) {
        case 2: {
            WRITE_LO(sim.tmp16, mem_read(sim, sim.pc + 1));
        } break;
        case 3: {
            WRITE_HI(sim.tmp16, mem_read(sim, sim.pc + 2));
        } break;
        case 4: {
             /* discard read */
        } break;
        case 5: {
            sim.tmp8 = mem_read(sim, sim.tmp16 + ((uint16)sim.x));
        } break;
        case 6: {
             /* discard unmodified write */
        } break;
        case 7: {
            sim.tmp8 -= 1;
            setnz(sim, sim.tmp8);
            mem_write(sim, sim.tmp16 + ((uint16)sim.x), sim.tmp8);
            sim.cycle = 1;
            sim.pc += 3;
        } break;
        
    }
}
void
op_cpx_imm(Sim6502 &sim)
{
    switch (sim.cycle) {
        case 2: {
            sim.tmp8 = mem_read(sim, sim.pc + 1);
        } break;
        case 3: {
            sim.tmp8 = sim.x - sim.tmp8;
            setnz(sim, sim.tmp8);
            sim.pflags.carry = sim.x >= sim.tmp8;
            sim.cycle = 1;
            sim.pc += 2;
        } break;
        
    }
}
void
op_sbc_izx(Sim6502 &sim)
{
    switch (sim.cycle) {
        case 2: {
            sim.tmp16 = mem_read(sim, sim.pc + 1);
        } break;
        case 3: {
            sim.tmp16 = (sim.tmp16 + sim.x) & 0xFF;
        } break;
        case 4: {
            sim.tmp8 = mem_read(sim, sim.tmp16 + 0);
        } break;
        case 5: {
            WRITE_HI(sim.tmp16, mem_read(sim, sim.tmp16 + 1));
        } break;
        case 6: {
            WRITE_LO(sim.tmp16, sim.tmp8);
        } break;
        case 7: {
            sim.tmp8 = mem_read(sim, sim.tmp16);
            sim.tmp8 = ~sim.tmp8;
            sim.tmp16 = 0;
            sim.tmp16 += sim.acc;
            sim.tmp16 += sim.tmp8;
            sim.tmp16 += sim.pflags.carry;            
            
            if (sim.pflags.deci) 
            { 
                sim.tmp8 = 0x99 - ~sim.tmp8;
                sim.tmp16 = (sim.acc & 0x0F) + (sim.tmp8 & 0x0F) + sim.pflags.carry;
        
                if (sim.tmp16 > 0x09) {
                    sim.tmp16 += 0x06;
                }

                sim.tmp16 += (sim.acc & 0xF0) + (sim.tmp8 & 0xF0);
                if (sim.tmp16 > 0x99) {
                    sim.tmp16 += 0x60;
                    sim.pflags.carry = 1;
                } else {
                    sim.pflags.carry = 0;
                }
            } // SBC BCD operation

            sim.pflags.ovrf = ((sim.acc & 0x80) == (sim.tmp8 & 0x80)) && 
                                              ((sim.acc & 0x80) != (sim.tmp16 & 0x80)) /* same sign operands, different sign result */;
            sim.pflags.carry = sim.tmp16 > 0xFF;
            sim.acc = READ_LO(sim.tmp16);
            setnz(sim, sim.acc);
            sim.cycle = 1;
            sim.pc += 2;
        } break;
        
    }
}
void
op_cpx_zp(Sim6502 &sim)
{
    switch (sim.cycle) {
        case 2: {
            sim.tmp16 = ((uint16)mem_read(sim, sim.pc + 1));
        } break;
        case 3: {
            sim.tmp8 = mem_read(sim, sim.tmp16);
        } break;
        case 4: {
            sim.tmp8 = sim.x - sim.tmp8;
            setnz(sim, sim.tmp8);
            sim.pflags.carry = sim.x >= sim.tmp8;
            sim.cycle = 1;
            sim.pc += 2;
        } break;
        
    }
}
void
op_sbc_zp(Sim6502 &sim)
{
    switch (sim.cycle) {
        case 2: {
            sim.tmp16 = ((uint16)mem_read(sim, sim.pc + 1));
        } break;
        case 3: {
            sim.tmp8 = mem_read(sim, sim.tmp16);
        } break;
        case 4: {
            sim.tmp8 = ~sim.tmp8;
            sim.tmp16 = 0;
            sim.tmp16 += sim.acc;
            sim.tmp16 += sim.tmp8;
            sim.tmp16 += sim.pflags.carry;            
            
            if (sim.pflags.deci) 
            { 
                sim.tmp8 = 0x99 - ~sim.tmp8;
                sim.tmp16 = (sim.acc & 0x0F) + (sim.tmp8 & 0x0F) + sim.pflags.carry;
        
                if (sim.tmp16 > 0x09) {
                    sim.tmp16 += 0x06;
                }

                sim.tmp16 += (sim.acc & 0xF0) + (sim.tmp8 & 0xF0);
                if (sim.tmp16 > 0x99) {
                    sim.tmp16 += 0x60;
                    sim.pflags.carry = 1;
                } else {
                    sim.pflags.carry = 0;
                }
            } // SBC BCD operation

            sim.pflags.ovrf = ((sim.acc & 0x80) == (sim.tmp8 & 0x80)) && 
                                              ((sim.acc & 0x80) != (sim.tmp16 & 0x80)) /* same sign operands, different sign result */;
            sim.pflags.carry = sim.tmp16 > 0xFF;
            sim.acc = READ_LO(sim.tmp16);
            setnz(sim, sim.acc);
            sim.cycle = 1;
            sim.pc += 2;
        } break;
        
    }
}
void
op_inc_zp(Sim6502 &sim)
{
    switch (sim.cycle) {
        case 2: {
            sim.tmp16 = ((uint16)mem_read(sim, sim.pc + 1));
        } break;
        case 3: {
            sim.tmp8 = mem_read(sim, sim.tmp16);
        } break;
        case 4: {
             /* discard read memwrite ADL */
        } break;
        case 5: {
            sim.tmp8 += 1;
            setnz(sim, sim.tmp8);
            mem_write(sim, sim.tmp16, sim.tmp8);
            sim.cycle = 1;
            sim.pc += 2;
        } break;
        
    }
}
void
op_inx_imp(Sim6502 &sim)
{
    switch (sim.cycle) {
        case 2: {
            sim.x += 1;
            setnz(sim, sim.x);
            sim.cycle = 1;
            sim.pc += 1;
        } break;
        
    }
}
void
op_sbc_imm(Sim6502 &sim)
{
    switch (sim.cycle) {
        case 2: {
            sim.tmp8 = mem_read(sim, sim.pc + 1);
        } break;
        case 3: {
            sim.tmp8 = ~sim.tmp8;
            sim.tmp16 = 0;
            sim.tmp16 += sim.acc;
            sim.tmp16 += sim.tmp8;
            sim.tmp16 += sim.pflags.carry;            
            
            if (sim.pflags.deci) 
            { 
                sim.tmp8 = 0x99 - ~sim.tmp8;
                sim.tmp16 = (sim.acc & 0x0F) + (sim.tmp8 & 0x0F) + sim.pflags.carry;
        
                if (sim.tmp16 > 0x09) {
                    sim.tmp16 += 0x06;
                }

                sim.tmp16 += (sim.acc & 0xF0) + (sim.tmp8 & 0xF0);
                if (sim.tmp16 > 0x99) {
                    sim.tmp16 += 0x60;
                    sim.pflags.carry = 1;
                } else {
                    sim.pflags.carry = 0;
                }
            } // SBC BCD operation

            sim.pflags.ovrf = ((sim.acc & 0x80) == (sim.tmp8 & 0x80)) && 
                                              ((sim.acc & 0x80) != (sim.tmp16 & 0x80)) /* same sign operands, different sign result */;
            sim.pflags.carry = sim.tmp16 > 0xFF;
            sim.acc = READ_LO(sim.tmp16);
            setnz(sim, sim.acc);
            sim.cycle = 1;
            sim.pc += 2;
        } break;
        
    }
}
void
op_nop_imp(Sim6502 &sim)
{
    switch (sim.cycle) {
        case 2: {
            /* NOP!!! */ ;
            sim.cycle = 1;
            sim.pc += 1;
        } break;
        
    }
}
void
op_cpx_abs(Sim6502 &sim)
{
    switch (sim.cycle) {
        case 2: {
            WRITE_LO(sim.tmp16, mem_read(sim, sim.pc + 1));
        } break;
        case 3: {
            WRITE_HI(sim.tmp16, mem_read(sim, sim.pc + 2));
        } break;
        case 4: {
            sim.tmp8 = mem_read(sim, sim.tmp16 + ((uint16)0));
        } break;
        case 5: {
            sim.tmp8 = sim.x - sim.tmp8;
            setnz(sim, sim.tmp8);
            sim.pflags.carry = sim.x >= sim.tmp8;
            sim.cycle = 1;
            sim.pc += 3;
        } break;
        
    }
}
void
op_sbc_abs(Sim6502 &sim)
{
    switch (sim.cycle) {
        case 2: {
            WRITE_LO(sim.tmp16, mem_read(sim, sim.pc + 1));
        } break;
        case 3: {
            WRITE_HI(sim.tmp16, mem_read(sim, sim.pc + 2));
        } break;
        case 4: {
            sim.tmp8 = mem_read(sim, sim.tmp16 + ((uint16)0));
        } break;
        case 5: {
            sim.tmp8 = ~sim.tmp8;
            sim.tmp16 = 0;
            sim.tmp16 += sim.acc;
            sim.tmp16 += sim.tmp8;
            sim.tmp16 += sim.pflags.carry;            
            
            if (sim.pflags.deci) 
            { 
                sim.tmp8 = 0x99 - ~sim.tmp8;
                sim.tmp16 = (sim.acc & 0x0F) + (sim.tmp8 & 0x0F) + sim.pflags.carry;
        
                if (sim.tmp16 > 0x09) {
                    sim.tmp16 += 0x06;
                }

                sim.tmp16 += (sim.acc & 0xF0) + (sim.tmp8 & 0xF0);
                if (sim.tmp16 > 0x99) {
                    sim.tmp16 += 0x60;
                    sim.pflags.carry = 1;
                } else {
                    sim.pflags.carry = 0;
                }
            } // SBC BCD operation

            sim.pflags.ovrf = ((sim.acc & 0x80) == (sim.tmp8 & 0x80)) && 
                                              ((sim.acc & 0x80) != (sim.tmp16 & 0x80)) /* same sign operands, different sign result */;
            sim.pflags.carry = sim.tmp16 > 0xFF;
            sim.acc = READ_LO(sim.tmp16);
            setnz(sim, sim.acc);
            sim.cycle = 1;
            sim.pc += 3;
        } break;
        
    }
}
void
op_inc_abs(Sim6502 &sim)
{
    switch (sim.cycle) {
        case 2: {
            WRITE_LO(sim.tmp16, mem_read(sim, sim.pc + 1));
        } break;
        case 3: {
            WRITE_HI(sim.tmp16, mem_read(sim, sim.pc + 2));
        } break;
        case 4: {
            sim.tmp8 = mem_read(sim, sim.tmp16 + ((uint16)0));
        } break;
        case 5: {
             /* discard unmodified write */
        } break;
        case 6: {
            sim.tmp8 += 1;
            setnz(sim, sim.tmp8);
            mem_write(sim, sim.tmp16 + ((uint16)0), sim.tmp8);
            sim.cycle = 1;
            sim.pc += 3;
        } break;
        
    }
}
void
op_beq_rel(Sim6502 &sim)
{
    switch (sim.cycle) {
        case 2: {
            // test the opposite of the operation
            if ( !(sim.pflags.zero == 1) ) { sim.cycle = 1; sim.pc += 2; }
            
            // break on test methods that sucessfully branch to itself
            else if (mem_read(sim, sim.pc + 1) == 0xFE) { init_gui_debugging(sim); sim.exec_mode = ExecMode_Pause; }
        } break;
        case 3: {
            sim.tmp16 = mem_read(sim, sim.pc + 1);
            if (sim.tmp16 & 0x80) sim.tmp16 |= 0xFF00; // sign extend
            sim.tmp16 += sim.pc + 2;
            bool overflow = READ_HI(sim.pc + 2) != READ_HI(sim.tmp16);
            sim.pc = sim.tmp16;
            if (!overflow) { sim.cycle = 1; } // page boundary cross check
        } break;
        case 4: {
             /* discard increment high byte of T16 */
            sim.cycle = 1;
        } break;
        
    }
}
void
op_sbc_izy(Sim6502 &sim)
{
    switch (sim.cycle) {
        case 2: {
            sim.tmp16 = mem_read(sim, sim.pc + 1);
        } break;
        case 3: {
            sim.tmp8 = mem_read(sim, sim.tmp16 + 0);
        } break;
        case 4: {
            WRITE_HI(sim.tmp16, mem_read(sim, sim.tmp16 + 1));
        } break;
        case 5: {
            WRITE_LO(sim.tmp16, sim.tmp8);
        } break;
        case 6: {
            sim.tmp16 += sim.y;
        } break;
        case 7: {
            sim.tmp8 = mem_read(sim, sim.tmp16);
            sim.tmp8 = ~sim.tmp8;
            sim.tmp16 = 0;
            sim.tmp16 += sim.acc;
            sim.tmp16 += sim.tmp8;
            sim.tmp16 += sim.pflags.carry;            
            
            if (sim.pflags.deci) 
            { 
                sim.tmp8 = 0x99 - ~sim.tmp8;
                sim.tmp16 = (sim.acc & 0x0F) + (sim.tmp8 & 0x0F) + sim.pflags.carry;
        
                if (sim.tmp16 > 0x09) {
                    sim.tmp16 += 0x06;
                }

                sim.tmp16 += (sim.acc & 0xF0) + (sim.tmp8 & 0xF0);
                if (sim.tmp16 > 0x99) {
                    sim.tmp16 += 0x60;
                    sim.pflags.carry = 1;
                } else {
                    sim.pflags.carry = 0;
                }
            } // SBC BCD operation

            sim.pflags.ovrf = ((sim.acc & 0x80) == (sim.tmp8 & 0x80)) && 
                                              ((sim.acc & 0x80) != (sim.tmp16 & 0x80)) /* same sign operands, different sign result */;
            sim.pflags.carry = sim.tmp16 > 0xFF;
            sim.acc = READ_LO(sim.tmp16);
            setnz(sim, sim.acc);
            sim.cycle = 1;
            sim.pc += 2;
        } break;
        
    }
}
void
op_sbc_zpx(Sim6502 &sim)
{
    switch (sim.cycle) {
        case 2: {
            sim.tmp8 = mem_read(sim, sim.pc + 1);
        } break;
        case 3: {
             /* discard memread BAL */
        } break;
        case 4: {
            sim.tmp16 = (sim.x + sim.tmp8) & 0xFF;
        } break;
        case 5: {
            sim.tmp8 = mem_read(sim, sim.tmp16);
        } break;
        case 6: {
            sim.tmp8 = ~sim.tmp8;
            sim.tmp16 = 0;
            sim.tmp16 += sim.acc;
            sim.tmp16 += sim.tmp8;
            sim.tmp16 += sim.pflags.carry;            
            
            if (sim.pflags.deci) 
            { 
                sim.tmp8 = 0x99 - ~sim.tmp8;
                sim.tmp16 = (sim.acc & 0x0F) + (sim.tmp8 & 0x0F) + sim.pflags.carry;
        
                if (sim.tmp16 > 0x09) {
                    sim.tmp16 += 0x06;
                }

                sim.tmp16 += (sim.acc & 0xF0) + (sim.tmp8 & 0xF0);
                if (sim.tmp16 > 0x99) {
                    sim.tmp16 += 0x60;
                    sim.pflags.carry = 1;
                } else {
                    sim.pflags.carry = 0;
                }
            } // SBC BCD operation

            sim.pflags.ovrf = ((sim.acc & 0x80) == (sim.tmp8 & 0x80)) && 
                                              ((sim.acc & 0x80) != (sim.tmp16 & 0x80)) /* same sign operands, different sign result */;
            sim.pflags.carry = sim.tmp16 > 0xFF;
            sim.acc = READ_LO(sim.tmp16);
            setnz(sim, sim.acc);
            sim.cycle = 1;
            sim.pc += 2;
        } break;
        
    }
}
void
op_inc_zpx(Sim6502 &sim)
{
    switch (sim.cycle) {
        case 2: {
            sim.tmp8 = mem_read(sim, sim.pc + 1);
        } break;
        case 3: {
             /* discard memread BAL */
        } break;
        case 4: {
            sim.tmp16 = (sim.x + sim.tmp8) & 0xFF;
        } break;
        case 5: {
            sim.tmp8 = mem_read(sim, sim.tmp16);
        } break;
        case 6: {
             /* discard unmodified write */
        } break;
        case 7: {
            sim.tmp8 += 1;
            setnz(sim, sim.tmp8);
            mem_write(sim, sim.tmp16, sim.tmp8);
            sim.cycle = 1;
            sim.pc += 2;
        } break;
        
    }
}
void
op_sed_imp(Sim6502 &sim)
{
    switch (sim.cycle) {
        case 2: {
            sim.pflags.deci = 1;
            sim.cycle = 1;
            sim.pc += 1;
        } break;
        
    }
}
void
op_sbc_absy(Sim6502 &sim)
{
    switch (sim.cycle) {
        case 2: {
            WRITE_LO(sim.tmp16, mem_read(sim, sim.pc + 1));
        } break;
        case 3: {
            WRITE_HI(sim.tmp16, mem_read(sim, sim.pc + 2));
        } break;
        case 4: {
            sim.tmp8 = mem_read(sim, sim.tmp16 + ((uint16)sim.y));
        } break;
        case 5: {
            sim.tmp8 = ~sim.tmp8;
            sim.tmp16 = 0;
            sim.tmp16 += sim.acc;
            sim.tmp16 += sim.tmp8;
            sim.tmp16 += sim.pflags.carry;            
            
            if (sim.pflags.deci) 
            { 
                sim.tmp8 = 0x99 - ~sim.tmp8;
                sim.tmp16 = (sim.acc & 0x0F) + (sim.tmp8 & 0x0F) + sim.pflags.carry;
        
                if (sim.tmp16 > 0x09) {
                    sim.tmp16 += 0x06;
                }

                sim.tmp16 += (sim.acc & 0xF0) + (sim.tmp8 & 0xF0);
                if (sim.tmp16 > 0x99) {
                    sim.tmp16 += 0x60;
                    sim.pflags.carry = 1;
                } else {
                    sim.pflags.carry = 0;
                }
            } // SBC BCD operation

            sim.pflags.ovrf = ((sim.acc & 0x80) == (sim.tmp8 & 0x80)) && 
                                              ((sim.acc & 0x80) != (sim.tmp16 & 0x80)) /* same sign operands, different sign result */;
            sim.pflags.carry = sim.tmp16 > 0xFF;
            sim.acc = READ_LO(sim.tmp16);
            setnz(sim, sim.acc);
            sim.cycle = 1;
            sim.pc += 3;
        } break;
        
    }
}
void
op_sbc_absx(Sim6502 &sim)
{
    switch (sim.cycle) {
        case 2: {
            WRITE_LO(sim.tmp16, mem_read(sim, sim.pc + 1));
        } break;
        case 3: {
            WRITE_HI(sim.tmp16, mem_read(sim, sim.pc + 2));
        } break;
        case 4: {
            sim.tmp8 = mem_read(sim, sim.tmp16 + ((uint16)sim.x));
        } break;
        case 5: {
            sim.tmp8 = ~sim.tmp8;
            sim.tmp16 = 0;
            sim.tmp16 += sim.acc;
            sim.tmp16 += sim.tmp8;
            sim.tmp16 += sim.pflags.carry;            
            
            if (sim.pflags.deci) 
            { 
                sim.tmp8 = 0x99 - ~sim.tmp8;
                sim.tmp16 = (sim.acc & 0x0F) + (sim.tmp8 & 0x0F) + sim.pflags.carry;
        
                if (sim.tmp16 > 0x09) {
                    sim.tmp16 += 0x06;
                }

                sim.tmp16 += (sim.acc & 0xF0) + (sim.tmp8 & 0xF0);
                if (sim.tmp16 > 0x99) {
                    sim.tmp16 += 0x60;
                    sim.pflags.carry = 1;
                } else {
                    sim.pflags.carry = 0;
                }
            } // SBC BCD operation

            sim.pflags.ovrf = ((sim.acc & 0x80) == (sim.tmp8 & 0x80)) && 
                                              ((sim.acc & 0x80) != (sim.tmp16 & 0x80)) /* same sign operands, different sign result */;
            sim.pflags.carry = sim.tmp16 > 0xFF;
            sim.acc = READ_LO(sim.tmp16);
            setnz(sim, sim.acc);
            sim.cycle = 1;
            sim.pc += 3;
        } break;
        
    }
}
void
op_inc_absx(Sim6502 &sim)
{
    switch (sim.cycle) {
        case 2: {
            WRITE_LO(sim.tmp16, mem_read(sim, sim.pc + 1));
        } break;
        case 3: {
            WRITE_HI(sim.tmp16, mem_read(sim, sim.pc + 2));
        } break;
        case 4: {
             /* discard read */
        } break;
        case 5: {
            sim.tmp8 = mem_read(sim, sim.tmp16 + ((uint16)sim.x));
        } break;
        case 6: {
             /* discard unmodified write */
        } break;
        case 7: {
            sim.tmp8 += 1;
            setnz(sim, sim.tmp8);
            mem_write(sim, sim.tmp16 + ((uint16)sim.x), sim.tmp8);
            sim.cycle = 1;
            sim.pc += 3;
        } break;
        
    }
}
void
init_opcodes(Opcode (&optable)[OPCODE_COUNT])
{
    for (Opcode &c: optable) { c = OPCODE_ILLEGAL; }

    optable[0x00] = {"BRK", op_brk_imp, AddrMode_Imp };
    optable[0x01] = {"ORA", op_ora_izx, AddrMode_Izx };
    //     optable[0x03] = {"SLO", op_slo_izx, AddrMode_Izx };
    //     optable[0x04] = {"NOP", op_nop_zp, AddrMode_Zp };
    optable[0x05] = {"ORA", op_ora_zp, AddrMode_Zp };
    optable[0x06] = {"ASL", op_asl_zp, AddrMode_Zp };
    //     optable[0x07] = {"SLO", op_slo_zp, AddrMode_Zp };
    optable[0x08] = {"PHP", op_php_imp, AddrMode_Imp };
    optable[0x09] = {"ORA", op_ora_imm, AddrMode_Imm };
    optable[0x0A] = {"ASL", op_asl_acc, AddrMode_Acc };
    //     optable[0x0B] = {"ANC", op_anc_imm, AddrMode_Imm };
    //     optable[0x0C] = {"NOP", op_nop_abs, AddrMode_Abs };
    optable[0x0D] = {"ORA", op_ora_abs, AddrMode_Abs };
    optable[0x0E] = {"ASL", op_asl_abs, AddrMode_Abs };
    //     optable[0x0F] = {"SLO", op_slo_abs, AddrMode_Abs };
    optable[0x10] = {"BPL", op_bpl_rel, AddrMode_Rel };
    optable[0x11] = {"ORA", op_ora_izy, AddrMode_Izy };
    //     optable[0x13] = {"SLO", op_slo_izy, AddrMode_Izy };
    //     optable[0x14] = {"NOP", op_nop_zpx, AddrMode_ZpX };
    optable[0x15] = {"ORA", op_ora_zpx, AddrMode_ZpX };
    optable[0x16] = {"ASL", op_asl_zpx, AddrMode_ZpX };
    //     optable[0x17] = {"SLO", op_slo_zpx, AddrMode_ZpX };
    optable[0x18] = {"CLC", op_clc_imp, AddrMode_Imp };
    optable[0x19] = {"ORA", op_ora_absy, AddrMode_AbsY };
    //     optable[0x1A] = {"NOP", op_nop_imp, AddrMode_Imp };
    //     optable[0x1B] = {"SLO", op_slo_absy, AddrMode_AbsY };
    //     optable[0x1C] = {"NOP", op_nop_absx, AddrMode_AbsX };
    optable[0x1D] = {"ORA", op_ora_absx, AddrMode_AbsX };
    optable[0x1E] = {"ASL", op_asl_absx, AddrMode_AbsX };
    //     optable[0x1F] = {"SLO", op_slo_absx, AddrMode_AbsX };
    optable[0x20] = {"JSR", op_jsr_abs, AddrMode_Abs };
    optable[0x21] = {"AND", op_and_izx, AddrMode_Izx };
    //     optable[0x23] = {"RLA", op_rla_izx, AddrMode_Izx };
    optable[0x24] = {"BIT", op_bit_zp, AddrMode_Zp };
    optable[0x25] = {"AND", op_and_zp, AddrMode_Zp };
    optable[0x26] = {"ROL", op_rol_zp, AddrMode_Zp };
    //     optable[0x27] = {"RLA", op_rla_zp, AddrMode_Zp };
    optable[0x28] = {"PLP", op_plp_imp, AddrMode_Imp };
    optable[0x29] = {"AND", op_and_imm, AddrMode_Imm };
    optable[0x2A] = {"ROL", op_rol_acc, AddrMode_Acc };
    //     optable[0x2B] = {"ANC", op_anc_imm, AddrMode_Imm };
    optable[0x2C] = {"BIT", op_bit_abs, AddrMode_Abs };
    optable[0x2D] = {"AND", op_and_abs, AddrMode_Abs };
    optable[0x2E] = {"ROL", op_rol_abs, AddrMode_Abs };
    //     optable[0x2F] = {"RLA", op_rla_abs, AddrMode_Abs };
    optable[0x30] = {"BMI", op_bmi_rel, AddrMode_Rel };
    optable[0x31] = {"AND", op_and_izy, AddrMode_Izy };
    //     optable[0x33] = {"RLA", op_rla_izy, AddrMode_Izy };
    //     optable[0x34] = {"NOP", op_nop_zpx, AddrMode_ZpX };
    optable[0x35] = {"AND", op_and_zpx, AddrMode_ZpX };
    optable[0x36] = {"ROL", op_rol_zpx, AddrMode_ZpX };
    //     optable[0x37] = {"RLA", op_rla_zpx, AddrMode_ZpX };
    optable[0x38] = {"SEC", op_sec_imp, AddrMode_Imp };
    optable[0x39] = {"AND", op_and_absy, AddrMode_AbsY };
    //     optable[0x3A] = {"NOP", op_nop_imp, AddrMode_Imp };
    //     optable[0x3B] = {"RLA", op_rla_absy, AddrMode_AbsY };
    //     optable[0x3C] = {"NOP", op_nop_absx, AddrMode_AbsX };
    optable[0x3D] = {"AND", op_and_absx, AddrMode_AbsX };
    optable[0x3E] = {"ROL", op_rol_absx, AddrMode_AbsX };
    //     optable[0x3F] = {"RLA", op_rla_absx, AddrMode_AbsX };
    optable[0x40] = {"RTI", op_rti_imp, AddrMode_Imp };
    optable[0x41] = {"EOR", op_eor_izx, AddrMode_Izx };
    //     optable[0x43] = {"SRE", op_sre_izx, AddrMode_Izx };
    //     optable[0x44] = {"NOP", op_nop_zp, AddrMode_Zp };
    optable[0x45] = {"EOR", op_eor_zp, AddrMode_Zp };
    optable[0x46] = {"LSR", op_lsr_zp, AddrMode_Zp };
    //     optable[0x47] = {"SRE", op_sre_zp, AddrMode_Zp };
    optable[0x48] = {"PHA", op_pha_imp, AddrMode_Imp };
    optable[0x49] = {"EOR", op_eor_imm, AddrMode_Imm };
    optable[0x4A] = {"LSR", op_lsr_acc, AddrMode_Acc };
    //     optable[0x4B] = {"ALR", op_alr_imm, AddrMode_Imm };
    optable[0x4C] = {"JMP", op_jmp_abs, AddrMode_Abs };
    optable[0x4D] = {"EOR", op_eor_abs, AddrMode_Abs };
    optable[0x4E] = {"LSR", op_lsr_abs, AddrMode_Abs };
    //     optable[0x4F] = {"SRE", op_sre_abs, AddrMode_Abs };
    optable[0x50] = {"BVC", op_bvc_rel, AddrMode_Rel };
    optable[0x51] = {"EOR", op_eor_izy, AddrMode_Izy };
    //     optable[0x53] = {"SRE", op_sre_izy, AddrMode_Izy };
    //     optable[0x54] = {"NOP", op_nop_zpx, AddrMode_ZpX };
    optable[0x55] = {"EOR", op_eor_zpx, AddrMode_ZpX };
    optable[0x56] = {"LSR", op_lsr_zpx, AddrMode_ZpX };
    //     optable[0x57] = {"SRE", op_sre_zpx, AddrMode_ZpX };
    optable[0x58] = {"CLI", op_cli_imp, AddrMode_Imp };
    optable[0x59] = {"EOR", op_eor_absy, AddrMode_AbsY };
    //     optable[0x5A] = {"NOP", op_nop_imp, AddrMode_Imp };
    //     optable[0x5B] = {"SRE", op_sre_absy, AddrMode_AbsY };
    //     optable[0x5C] = {"NOP", op_nop_absx, AddrMode_AbsX };
    optable[0x5D] = {"EOR", op_eor_absx, AddrMode_AbsX };
    optable[0x5E] = {"LSR", op_lsr_absx, AddrMode_AbsX };
    //     optable[0x5F] = {"SRE", op_sre_absx, AddrMode_AbsX };
    optable[0x60] = {"RTS", op_rts_imp, AddrMode_Imp };
    optable[0x61] = {"ADC", op_adc_izx, AddrMode_Izx };
    //     optable[0x63] = {"RRA", op_rra_izx, AddrMode_Izx };
    //     optable[0x64] = {"NOP", op_nop_zp, AddrMode_Zp };
    optable[0x65] = {"ADC", op_adc_zp, AddrMode_Zp };
    optable[0x66] = {"ROR", op_ror_zp, AddrMode_Zp };
    //     optable[0x67] = {"RRA", op_rra_zp, AddrMode_Zp };
    optable[0x68] = {"PLA", op_pla_imp, AddrMode_Imp };
    optable[0x69] = {"ADC", op_adc_imm, AddrMode_Imm };
    optable[0x6A] = {"ROR", op_ror_acc, AddrMode_Acc };
    //     optable[0x6B] = {"ARR", op_arr_imm, AddrMode_Imm };
    optable[0x6C] = {"JMP", op_jmp_idr, AddrMode_Idr };
    optable[0x6D] = {"ADC", op_adc_abs, AddrMode_Abs };
    optable[0x6E] = {"ROR", op_ror_abs, AddrMode_Abs };
    //     optable[0x6F] = {"RRA", op_rra_abs, AddrMode_Abs };
    optable[0x70] = {"BVS", op_bvs_rel, AddrMode_Rel };
    optable[0x71] = {"ADC", op_adc_izy, AddrMode_Izy };
    //     optable[0x73] = {"RRA", op_rra_izy, AddrMode_Izy };
    //     optable[0x74] = {"NOP", op_nop_zpx, AddrMode_ZpX };
    optable[0x75] = {"ADC", op_adc_zpx, AddrMode_ZpX };
    optable[0x76] = {"ROR", op_ror_zpx, AddrMode_ZpX };
    //     optable[0x77] = {"RRA", op_rra_zpx, AddrMode_ZpX };
    optable[0x78] = {"SEI", op_sei_imp, AddrMode_Imp };
    optable[0x79] = {"ADC", op_adc_absy, AddrMode_AbsY };
    //     optable[0x7A] = {"NOP", op_nop_imp, AddrMode_Imp };
    //     optable[0x7B] = {"RRA", op_rra_absy, AddrMode_AbsY };
    //     optable[0x7C] = {"NOP", op_nop_absx, AddrMode_AbsX };
    optable[0x7D] = {"ADC", op_adc_absx, AddrMode_AbsX };
    optable[0x7E] = {"ROR", op_ror_absx, AddrMode_AbsX };
    //     optable[0x7F] = {"RRA", op_rra_absx, AddrMode_AbsX };
    //     optable[0x80] = {"NOP", op_nop_imm, AddrMode_Imm };
    optable[0x81] = {"STA", op_sta_izx, AddrMode_Izx };
    //     optable[0x82] = {"NOP", op_nop_imm, AddrMode_Imm };
    //     optable[0x83] = {"SAX", op_sax_izx, AddrMode_Izx };
    optable[0x84] = {"STY", op_sty_zp, AddrMode_Zp };
    optable[0x85] = {"STA", op_sta_zp, AddrMode_Zp };
    optable[0x86] = {"STX", op_stx_zp, AddrMode_Zp };
    //     optable[0x87] = {"SAX", op_sax_zp, AddrMode_Zp };
    optable[0x88] = {"DEY", op_dey_imp, AddrMode_Imp };
    //     optable[0x89] = {"NOP", op_nop_imm, AddrMode_Imm };
    optable[0x8A] = {"TXA", op_txa_imp, AddrMode_Imp };
    //     optable[0x8B] = {"XAA", op_xaa_imm, AddrMode_Imm };
    optable[0x8C] = {"STY", op_sty_abs, AddrMode_Abs };
    optable[0x8D] = {"STA", op_sta_abs, AddrMode_Abs };
    optable[0x8E] = {"STX", op_stx_abs, AddrMode_Abs };
    //     optable[0x8F] = {"SAX", op_sax_abs, AddrMode_Abs };
    optable[0x90] = {"BCC", op_bcc_rel, AddrMode_Rel };
    optable[0x91] = {"STA", op_sta_izy, AddrMode_Izy };
    //     optable[0x93] = {"AHX", op_ahx_izy, AddrMode_Izy };
    optable[0x94] = {"STY", op_sty_zpx, AddrMode_ZpX };
    optable[0x95] = {"STA", op_sta_zpx, AddrMode_ZpX };
    optable[0x96] = {"STX", op_stx_zpy, AddrMode_ZpY };
    //     optable[0x97] = {"SAX", op_sax_zpy, AddrMode_ZpY };
    optable[0x98] = {"TYA", op_tya_imp, AddrMode_Imp };
    optable[0x99] = {"STA", op_sta_absy, AddrMode_AbsY };
    optable[0x9A] = {"TXS", op_txs_imp, AddrMode_Imp };
    //     optable[0x9B] = {"TAS", op_tas_absy, AddrMode_AbsY };
    //     optable[0x9C] = {"SHY", op_shy_absx, AddrMode_AbsX };
    optable[0x9D] = {"STA", op_sta_absx, AddrMode_AbsX };
    //     optable[0x9E] = {"SHX", op_shx_absy, AddrMode_AbsY };
    //     optable[0x9F] = {"AHX", op_ahx_absy, AddrMode_AbsY };
    optable[0xA0] = {"LDY", op_ldy_imm, AddrMode_Imm };
    optable[0xA1] = {"LDA", op_lda_izx, AddrMode_Izx };
    optable[0xA2] = {"LDX", op_ldx_imm, AddrMode_Imm };
    //     optable[0xA3] = {"LAX", op_lax_izx, AddrMode_Izx };
    optable[0xA4] = {"LDY", op_ldy_zp, AddrMode_Zp };
    optable[0xA5] = {"LDA", op_lda_zp, AddrMode_Zp };
    optable[0xA6] = {"LDX", op_ldx_zp, AddrMode_Zp };
    //     optable[0xA7] = {"LAX", op_lax_zp, AddrMode_Zp };
    optable[0xA8] = {"TAY", op_tay_imp, AddrMode_Imp };
    optable[0xA9] = {"LDA", op_lda_imm, AddrMode_Imm };
    optable[0xAA] = {"TAX", op_tax_imp, AddrMode_Imp };
    //     optable[0xAB] = {"LAX", op_lax_imm, AddrMode_Imm };
    optable[0xAC] = {"LDY", op_ldy_abs, AddrMode_Abs };
    optable[0xAD] = {"LDA", op_lda_abs, AddrMode_Abs };
    optable[0xAE] = {"LDX", op_ldx_abs, AddrMode_Abs };
    //     optable[0xAF] = {"LAX", op_lax_abs, AddrMode_Abs };
    optable[0xB0] = {"BCS", op_bcs_rel, AddrMode_Rel };
    optable[0xB1] = {"LDA", op_lda_izy, AddrMode_Izy };
    //     optable[0xB3] = {"LAX", op_lax_izy, AddrMode_Izy };
    optable[0xB4] = {"LDY", op_ldy_zpx, AddrMode_ZpX };
    optable[0xB5] = {"LDA", op_lda_zpx, AddrMode_ZpX };
    optable[0xB6] = {"LDX", op_ldx_zpy, AddrMode_ZpY };
    //     optable[0xB7] = {"LAX", op_lax_zpy, AddrMode_ZpY };
    optable[0xB8] = {"CLV", op_clv_imp, AddrMode_Imp };
    optable[0xB9] = {"LDA", op_lda_absy, AddrMode_AbsY };
    optable[0xBA] = {"TSX", op_tsx_imp, AddrMode_Imp };
    //     optable[0xBB] = {"LAS", op_las_absy, AddrMode_AbsY };
    optable[0xBC] = {"LDY", op_ldy_absx, AddrMode_AbsX };
    optable[0xBD] = {"LDA", op_lda_absx, AddrMode_AbsX };
    optable[0xBE] = {"LDX", op_ldx_absy, AddrMode_AbsY };
    //     optable[0xBF] = {"LAX", op_lax_absy, AddrMode_AbsY };
    optable[0xC0] = {"CPY", op_cpy_imm, AddrMode_Imm };
    optable[0xC1] = {"CMP", op_cmp_izx, AddrMode_Izx };
    //     optable[0xC2] = {"NOP", op_nop_imm, AddrMode_Imm };
    //     optable[0xC3] = {"DCP", op_dcp_izx, AddrMode_Izx };
    optable[0xC4] = {"CPY", op_cpy_zp, AddrMode_Zp };
    optable[0xC5] = {"CMP", op_cmp_zp, AddrMode_Zp };
    optable[0xC6] = {"DEC", op_dec_zp, AddrMode_Zp };
    //     optable[0xC7] = {"DCP", op_dcp_zp, AddrMode_Zp };
    optable[0xC8] = {"INY", op_iny_imp, AddrMode_Imp };
    optable[0xC9] = {"CMP", op_cmp_imm, AddrMode_Imm };
    optable[0xCA] = {"DEX", op_dex_imp, AddrMode_Imp };
    //     optable[0xCB] = {"AXS", op_axs_imm, AddrMode_Imm };
    optable[0xCC] = {"CPY", op_cpy_abs, AddrMode_Abs };
    optable[0xCD] = {"CMP", op_cmp_abs, AddrMode_Abs };
    optable[0xCE] = {"DEC", op_dec_abs, AddrMode_Abs };
    //     optable[0xCF] = {"DCP", op_dcp_abs, AddrMode_Abs };
    optable[0xD0] = {"BNE", op_bne_rel, AddrMode_Rel };
    optable[0xD1] = {"CMP", op_cmp_izy, AddrMode_Izy };
    //     optable[0xD3] = {"DCP", op_dcp_izy, AddrMode_Izy };
    //     optable[0xD4] = {"NOP", op_nop_zpx, AddrMode_ZpX };
    optable[0xD5] = {"CMP", op_cmp_zpx, AddrMode_ZpX };
    optable[0xD6] = {"DEC", op_dec_zpx, AddrMode_ZpX };
    //     optable[0xD7] = {"DCP", op_dcp_zpx, AddrMode_ZpX };
    optable[0xD8] = {"CLD", op_cld_imp, AddrMode_Imp };
    optable[0xD9] = {"CMP", op_cmp_absy, AddrMode_AbsY };
    //     optable[0xDA] = {"NOP", op_nop_imp, AddrMode_Imp };
    //     optable[0xDB] = {"DCP", op_dcp_absy, AddrMode_AbsY };
    //     optable[0xDC] = {"NOP", op_nop_absx, AddrMode_AbsX };
    optable[0xDD] = {"CMP", op_cmp_absx, AddrMode_AbsX };
    optable[0xDE] = {"DEC", op_dec_absx, AddrMode_AbsX };
    //     optable[0xDF] = {"DCP", op_dcp_absx, AddrMode_AbsX };
    optable[0xE0] = {"CPX", op_cpx_imm, AddrMode_Imm };
    optable[0xE1] = {"SBC", op_sbc_izx, AddrMode_Izx };
    //     optable[0xE2] = {"NOP", op_nop_imm, AddrMode_Imm };
    //     optable[0xE3] = {"ISC", op_isc_izx, AddrMode_Izx };
    optable[0xE4] = {"CPX", op_cpx_zp, AddrMode_Zp };
    optable[0xE5] = {"SBC", op_sbc_zp, AddrMode_Zp };
    optable[0xE6] = {"INC", op_inc_zp, AddrMode_Zp };
    //     optable[0xE7] = {"ISC", op_isc_zp, AddrMode_Zp };
    optable[0xE8] = {"INX", op_inx_imp, AddrMode_Imp };
    optable[0xE9] = {"SBC", op_sbc_imm, AddrMode_Imm };
    optable[0xEA] = {"NOP", op_nop_imp, AddrMode_Imp };
    //     optable[0xEB] = {"SBC", op_sbc_imp, AddrMode_Imp };
    optable[0xEC] = {"CPX", op_cpx_abs, AddrMode_Abs };
    optable[0xED] = {"SBC", op_sbc_abs, AddrMode_Abs };
    optable[0xEE] = {"INC", op_inc_abs, AddrMode_Abs };
    //     optable[0xEF] = {"ISC", op_isc_abs, AddrMode_Abs };
    optable[0xF0] = {"BEQ", op_beq_rel, AddrMode_Rel };
    optable[0xF1] = {"SBC", op_sbc_izy, AddrMode_Izy };
    //     optable[0xF3] = {"ISC", op_isc_izy, AddrMode_Izy };
    //     optable[0xF4] = {"NOP", op_nop_zpx, AddrMode_ZpX };
    optable[0xF5] = {"SBC", op_sbc_zpx, AddrMode_ZpX };
    optable[0xF6] = {"INC", op_inc_zpx, AddrMode_ZpX };
    //     optable[0xF7] = {"ISC", op_isc_zpx, AddrMode_ZpX };
    optable[0xF8] = {"SED", op_sed_imp, AddrMode_Imp };
    optable[0xF9] = {"SBC", op_sbc_absy, AddrMode_AbsY };
    //     optable[0xFA] = {"NOP", op_nop_imp, AddrMode_Imp };
    //     optable[0xFB] = {"ISC", op_isc_absy, AddrMode_AbsY };
    //     optable[0xFC] = {"NOP", op_nop_absx, AddrMode_AbsX };
    optable[0xFD] = {"SBC", op_sbc_absx, AddrMode_AbsX };
    optable[0xFE] = {"INC", op_inc_absx, AddrMode_AbsX };
    //     optable[0xFF] = {"ISC", op_isc_absx, AddrMode_AbsX };
}

