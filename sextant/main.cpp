#include <hal/multiboot.h>
#include <drivers/Ecran.h>
#include <drivers/PortSerie.h>

// TP2
#include <sextant/interruptions/idt.h>
#include <sextant/interruptions/irq.h>
#include <sextant/interruptions/handler/handler_tic.h>
#include <sextant/interruptions/handler/handler_clavier.h>
#include <drivers/timer.h>
#include <drivers/Clavier.h>
// TP3
#include <sextant/memoire/memoire.h>

// TP4
#include <sextant/ordonnancements/cpu_context.h>
#include <sextant/ordonnancements/preemptif/thread.h>
#include <sextant/types.h>


#include <sextant/Synchronisation/Spinlock/Spinlock.h>

#include <hal/pci.h>
#include <drivers/vga.h>
#include <drivers/EcranBochs.h>

#include <sextant/sprite.h>

#include <Applications/Breakout/Breakout.h>
#include <Applications/Breakout/BreakoutInput.h>
#include <Applications/Breakout/Ball.h>
#include <Applications/Breakout/Paddle.h>
#include <Applications/Breakout/Brick.h>
#include <sextant/Activite/Threads.h>


extern char __e_kernel,__b_kernel, __b_data, __e_data,  __b_stack, __e_load ;
int i;

extern vaddr_t bootstrap_stack_bottom; //Adresse de début de la pile d'exécution
extern size_t bootstrap_stack_size;//Taille de la pile d'exécution

void demo_vga() {
	set_vga_mode13(); // set VGA mode
	set_palette_vga(palette_vga); // set to given palette

	ui16_t offset = 0;
	while(1) {
		clear_vga_screen(0); // put the color 0 on each pixel
		plot_square(offset, 50, 25, 4); // plot a square of 25 width at 50,50 of color 4
		draw_sprite(sprite_door_data, 32, 32, 100,100); // draw the 32x32 sprite at 100,100
		offset = (offset + 1) % 640;
	}
}

// Global game instance for multi-threaded Breakout
Breakout* globalGame = nullptr;
EcranBochs* globalVGA = nullptr;
BreakoutInput* globalInput = nullptr;

// Keyboard callback function
void keyboardCallback(unsigned char scancode, bool pressed) {
	PortSerie ps;
	if (pressed) {
		ps.ecrireMot("Key press: ");
	} else {
		ps.ecrireMot("Key release: ");
	}
	
	// Simple scancode display
	char buf[10];
	int i = 0;
	int val = scancode;
	do {
		buf[i++] = '0' + (val % 10);
		val /= 10;
	} while (val > 0);
	buf[i] = '\0';
	
	// Reverse the string
	for (int j = 0; j < i/2; j++) {
		char tmp = buf[j];
		buf[j] = buf[i-j-1];
		buf[i-j-1] = tmp;
	}
	ps.ecrireMot(buf);
	ps.ecrireMot("\n");
	
	if (globalInput) {
		if (pressed) {
			globalInput->handleKeyPress(scancode);
		} else {
			globalInput->handleKeyRelease(scancode);
		}
	}
}

// Thread class for game logic
class GameLogicThread : public Threads {
public:
	void run() override {
		PortSerie ps;
		ps.ecrireMot("Game Logic Thread started\n");
		
		while (globalGame && globalGame->isGameRunning()) {
			globalGame->updateLogic();
			this->Yield();
		}
		
		ps.ecrireMot("Game Logic Thread finished\n");
	}
};

// Thread class for rendering
class RenderThread : public Threads {
public:
	void run() override {
		PortSerie ps;
		ps.ecrireMot("Render Thread started\n");
		
		while (globalGame && globalGame->isGameRunning()) {
			if (globalVGA) {
				// Render needs to read game state, but don't lock too long
				globalGame->render();
				globalVGA->swapBuffer();
			}
			// Yield more often to allow game logic to run
			this->Yield();
		}
		
		ps.ecrireMot("Render Thread finished\n");
	}
};

void demo_breakout_bochs() {
	PortSerie ps;
	ps.ecrireMot("Starting Breakout with threads...\n");
	
	// Initialize VGA
	globalVGA = new EcranBochs(640, 400, VBE_MODE::_8);
	globalVGA->init();
	globalVGA->clear(0);
	globalVGA->set_palette(palette_vga);
	ps.ecrireMot("VGA initialized\n");
	
	// Create game instance
	globalGame = new Breakout();
	globalGame->init();
	globalInput = new BreakoutInput(globalGame);
	ps.ecrireMot("Game created\n");
	
	// Register keyboard callback
	setKeyboardCallback(keyboardCallback);
	ps.ecrireMot("Keyboard callback registered\n");
	
	// Create and start threads
	ps.ecrireMot("Creating game threads...\n");
	
	GameLogicThread* gameThread = new GameLogicThread();
	RenderThread* renderThread = new RenderThread();
	
	ps.ecrireMot("Starting threads...\n");
	gameThread->start();
	renderThread->start();
	
	ps.ecrireMot("Threads started! Game running with:\n");
	ps.ecrireMot("- Player 1 (bottom): Q/D keys\n");
	ps.ecrireMot("- Player 2 (top): Arrow Left/Right keys\n");
	ps.ecrireMot("- Ordonnanceur: Preemptif Round-Robin\n");
	ps.ecrireMot("- Synchronisation: Mutex\n");
	
	// Main thread just waits
	while(1) {
		thread_yield();
	}
}

void demo_bochs_8() {
	EcranBochs vga(640, 400, VBE_MODE::_8);

    vga.init();
    vga.clear(0);

    // only usefull in 4 or 8 bits modes
    vga.set_palette(palette_vga);
    vga.plot_palette(0, 0, 25);

	int offset = 0;
	while (true) {
		vga.clear(1);
		vga.plot_sprite(sprite_data, SPRITE_WIDTH, SPRITE_HEIGHT, offset, 200);
		offset = (offset+1) % (640);
		vga.swapBuffer(); // call this after you finish drawing your frame to display it, it avoids screen tearing
	}
}

void demo_bochs_32() {
	EcranBochs vga(640, 400, VBE_MODE::_32);

	vga.init();
	
	ui8_t offset = 0;
	while(true) {
		
		for (int y = 0; y < vga.getHeight(); y++) {
			for (int x = 0; x < vga.getWidth(); x++) {
				vga.paint(x, y, 
					(~x << y%3) + offset & y, 
					~offset * (x & ~y), 
					offset | (~y < 2 - x % 16));
			}
		}
		++offset;
	}
}

extern "C" void Sextant_main(unsigned long magic, unsigned long addr){
	Ecran ecran;
	Timer timer;

	idt_setup();
	irq_setup();
	//Initialisation de la frequence de l'horloge

	timer.i8254_set_frequency(1000);
	irq_set_routine(IRQ_TIMER, ticTac);

	asm volatile("sti\n");//Autorise les interruptions

	irq_set_routine(IRQ_KEYBOARD, handler_clavier);

	multiboot_info_t* mbi;
	mbi = (multiboot_info_t*)addr;

	mem_setup(& __e_kernel,(mbi->mem_upper<<10) + (1<<20),&ecran);

	ecran.effacerEcran(NOIR);

	thread_subsystem_setup(bootstrap_stack_bottom,bootstrap_stack_size);
	sched_subsystem_setup();

	irq_set_routine(IRQ_TIMER, sched_clk);

	// initialize pci bus to detect GPU address
	checkBus(0);


	// demo_vga();

	// demo_bochs_8();
	
	demo_breakout_bochs();

	// demo_bochs_32();
}
