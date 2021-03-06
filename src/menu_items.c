/*
    ZEsarUX  ZX Second-Emulator And Released for UniX
    Copyright (C) 2013 Cesar Hernandez Bano

    This file is part of ZEsarUX.

    ZEsarUX is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <stdarg.h>
#include <dirent.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <errno.h>


#include "menu.h"
#include "menu_items.h"
#include "screen.h"
#include "cpu.h"
#include "debug.h"
#include "zx8081.h"
#include "ay38912.h"
#include "tape.h"
#include "audio.h"
#include "timer.h"
#include "snap.h"
#include "operaciones.h"
#include "disassemble.h"
#include "utils.h"
#include "contend.h"
#include "joystick.h"
#include "ula.h"
#include "printers.h"
#include "realjoystick.h"
#include "scrstdout.h"
#include "z88.h"
#include "ulaplus.h"
#include "autoselectoptions.h"
#include "zxuno.h"
#include "charset.h"
#include "chardetect.h"
#include "textspeech.h"
#include "mmc.h"
#include "ide.h"
#include "divmmc.h"
#include "divide.h"
#include "diviface.h"
#include "zxpand.h"
#include "spectra.h"
#include "spritechip.h"
#include "jupiterace.h"
#include "timex.h"
#include "chloe.h"
#include "prism.h"
#include "cpc.h"
#include "sam.h"
#include "atomlite.h"
#include "if1.h"
#include "pd765.h"
#include "tbblue.h"
#include "dandanator.h"
#include "superupgrade.h"
#include "m68k.h"
#include "remote.h"
#include "snap_rzx.h"
#include "multiface.h"
#include "scmp.h"
#include "esxdos_handler.h"
#include "tsconf.h"
#include "kartusho.h"
#include "spritefinder.h"
#include "snap_spg.h"
#include "betadisk.h"
#include "tape_tzx.h" 
#include "snap_zsf.h"
#include "compileoptions.h"
#include "settings.h"
#include "datagear.h"
#include "assemble.h"

 
#if defined(__APPLE__)
	#include <sys/syslimits.h>

	#include <sys/resource.h>

#endif


#ifdef COMPILE_CURSES
	#include "scrcurses.h"
#endif

#ifdef COMPILE_AA
	#include "scraa.h"
#endif

#ifdef COMPILE_STDOUT
	#include "scrstdout.h"
//macro llama a funcion real
	#define scrstdout_menu_print_speech_macro scrstdout_menu_print_speech
//funcion llama
#else
//funcion no llama a nada
	#define scrstdout_menu_print_speech_macro(x)
#endif


#ifdef COMPILE_XWINDOWS
	#include "scrxwindows.h"
#endif


//
// Archivo para entradas de menu, excluyendo funciones auxiliares de soporte de menu
// Las funciones auxiliares de menu estan en menu.c
// Aunque aun falta mucho por mover, la mayoria de entradas de menu siguen en menu.c y habria que moverlas aqui
//


//Opciones seleccionadas para cada menu
int debug_pok_file_opcion_seleccionada=0;
int poke_opcion_seleccionada=0; 
int settings_debug_opcion_seleccionada=0;
int change_audio_driver_opcion_seleccionada=0;
int settings_audio_opcion_seleccionada=0;
int mem_breakpoints_opcion_seleccionada=0;
int audio_new_waveform_opcion_seleccionada=0;
int debug_new_visualmem_opcion_seleccionada=0;
int audio_new_ayplayer_opcion_seleccionada=0;
int osd_adventure_keyboard_opcion_seleccionada=0;
int debug_tsconf_dma_opcion_seleccionada=0;
int tsconf_layer_settings_opcion_seleccionada=0;
int cpu_settings_opcion_seleccionada=0;
int textdrivers_settings_opcion_seleccionada=0;
int settings_display_opcion_seleccionada=0;
int cpu_stats_opcion_seleccionada=0;
int menu_tbblue_hardware_id_opcion_seleccionada=0;

//Fin opciones seleccionadas para cada menu


//Ultima direccion pokeada
int last_debug_poke_dir=16384; 

//aofile. aofilename apuntara aqui
char aofilename_file[PATH_MAX];

char last_ay_file[PATH_MAX]="";

//vofile. vofilename apuntara aqui
char vofilename_file[PATH_MAX];


void menu_mem_breakpoints_edit(MENU_ITEM_PARAMETERS)
{


        int brkp_type,dir;

        char string_type[4];
        char string_dir[10];

        strcpy (string_dir,"0");

        menu_ventana_scanf("Address",string_dir,10);

        dir=parse_string_to_number(string_dir);

        if (dir<0 || dir>65535) {
                debug_printf (VERBOSE_ERR,"Invalid address %d",dir);
                return;
        }				

        strcpy (string_type,"0");

        menu_ventana_scanf("Type (1:RD,2:WR,3:RW)",string_type,4);

        brkp_type=parse_string_to_number(string_type);

        if (brkp_type<0 || brkp_type>255) {
                debug_printf (VERBOSE_ERR,"Invalid value %d",brkp_type);
                return;
        }

	debug_set_mem_breakpoint(dir,brkp_type);
	//mem_breakpoint_array[dir]=brkp_type;
	

}

void menu_mem_breakpoints_list(MENU_ITEM_PARAMETERS)
{

        //int index_find;
		int index_buffer;

        char results_buffer[MAX_TEXTO_GENERIC_MESSAGE];

        //margen suficiente para que quepa una linea
        //direccion+salto linea+codigo 0
        char buf_linea[33];

        index_buffer=0;

        int encontrados=0;

        int salir=0;

		int i;

        for (i=0;i<65536 && salir==0;i++) {
			z80_byte tipo=mem_breakpoint_array[i];
			if (tipo) {
				if (tipo<MAX_MEM_BREAKPOINT_TYPES) {
					sprintf (buf_linea,"%04XH : %s\n",i,mem_breakpoint_types_strings[tipo]);
				}
				else {
					sprintf (buf_linea,"%04XH : Unknown (%d)\n",i,tipo);
				}

				sprintf (&results_buffer[index_buffer],"%s\n",buf_linea);
                index_buffer +=strlen(buf_linea);
                encontrados++;
                

                //controlar maximo
                //33 bytes de margen
                if (index_buffer>MAX_TEXTO_GENERIC_MESSAGE-33) {
                        debug_printf (VERBOSE_ERR,"Too many results to show. Showing only the first %d",encontrados);
                        //forzar salir
                        salir=1;
                }
			}

        }

        results_buffer[index_buffer]=0;

        menu_generic_message("List Memory Breakpoints",results_buffer);
}

void menu_mem_breakpoints_clear(MENU_ITEM_PARAMETERS)
{
	if (menu_confirm_yesno("Clear breakpoints")) {
		clear_mem_breakpoints();
		menu_generic_message("Clear breakpoints","OK. All breakpoints cleared");
	}
}


void menu_mem_breakpoints(MENU_ITEM_PARAMETERS)
{

	menu_espera_no_tecla();

        menu_item *array_menu_mem_breakpoints;
        menu_item item_seleccionado;
        int retorno_menu;
        do {


		menu_add_item_menu_inicial_format(&array_menu_mem_breakpoints,MENU_OPCION_NORMAL,menu_mem_breakpoints_edit,NULL,"~~Edit Breakpoint");
		menu_add_item_menu_shortcut(array_menu_mem_breakpoints,'e');
		menu_add_item_menu_tooltip(array_menu_mem_breakpoints,"Edit Breakpoints");
		menu_add_item_menu_ayuda(array_menu_mem_breakpoints,"Edit Breakpoints");

		menu_add_item_menu_format(array_menu_mem_breakpoints,MENU_OPCION_NORMAL,menu_mem_breakpoints_list,NULL,"~~List breakpoints");
		menu_add_item_menu_shortcut(array_menu_mem_breakpoints,'l');
		menu_add_item_menu_tooltip(array_menu_mem_breakpoints,"List breakpoints");
		menu_add_item_menu_ayuda(array_menu_mem_breakpoints,"List enabled memory breakpoints");


		menu_add_item_menu_format(array_menu_mem_breakpoints,MENU_OPCION_NORMAL,menu_mem_breakpoints_clear,NULL,"~~Clear breakpoints");
		menu_add_item_menu_shortcut(array_menu_mem_breakpoints,'c');
		menu_add_item_menu_tooltip(array_menu_mem_breakpoints,"Clear all memory breakpoints");
		menu_add_item_menu_ayuda(array_menu_mem_breakpoints,"Clear all memory breakpoints");


                menu_add_item_menu(array_menu_mem_breakpoints,"",MENU_OPCION_SEPARADOR,NULL,NULL);
                menu_add_ESC_item(array_menu_mem_breakpoints);
                retorno_menu=menu_dibuja_menu(&mem_breakpoints_opcion_seleccionada,&item_seleccionado,array_menu_mem_breakpoints,"Memory Breakpoints" );

                

                if ((item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu>=0) {
                        //llamamos por valor de funcion
                        if (item_seleccionado.menu_funcion!=NULL) {
                                //printf ("actuamos por funcion\n");
                                item_seleccionado.menu_funcion(item_seleccionado.valor_opcion);
                                
                        }
                }

        } while ( (item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu!=MENU_RETORNO_ESC && !salir_todos_menus);
}


 
void menu_debug_poke(MENU_ITEM_PARAMETERS)
{

        int valor_poke,dir,veces;

        char string_poke[4];
        char string_dir[10];
	char string_veces[6];

        sprintf (string_dir,"%XH",last_debug_poke_dir);

        menu_ventana_scanf("Address",string_dir,10);

        dir=parse_string_to_number(string_dir);

        /*if ( (dir<0 || dir>65535) && MACHINE_IS_SPECTRUM) {
                debug_printf (VERBOSE_ERR,"Invalid address %d",dir);
                return;
        }*/

				last_debug_poke_dir=dir;

        sprintf (string_poke,"0");

        menu_ventana_scanf("Poke Value",string_poke,4);

        valor_poke=parse_string_to_number(string_poke);

        if (valor_poke<0 || valor_poke>255) {
                debug_printf (VERBOSE_ERR,"Invalid value %d",valor_poke);
                return;
        }


	sprintf (string_veces,"1");

	menu_ventana_scanf("How many bytes?",string_veces,6);

	veces=parse_string_to_number(string_veces);

	if (veces<1 || veces>65536) {
                debug_printf (VERBOSE_ERR,"Invalid quantity %d",veces);
		return;
	}


	for (;veces;veces--,dir++) {

	        //poke_byte_no_time(dir,valor_poke);
		//poke_byte_z80_moto(dir,valor_poke);
		menu_debug_write_mapped_byte(dir,valor_poke);

	}

}



void menu_debug_poke_pok_file(MENU_ITEM_PARAMETERS)
{

        char *filtros[2];

        filtros[0]="pok";
        filtros[1]=0;

	char pokfile[PATH_MAX];

        int ret;

        ret=menu_filesel("Select POK File",filtros,pokfile);

	//contenido
	//MAX_LINEAS_POK_FILE es maximo de lineas de pok file
	//normalmente la tabla de pokes sera menor que el numero de lineas en el archivo .pok
	struct s_pokfile tabla_pokes[MAX_LINEAS_POK_FILE];

	//punteros
	struct s_pokfile *punteros_pokes[MAX_LINEAS_POK_FILE];

	int i;
	for (i=0;i<MAX_LINEAS_POK_FILE;i++) punteros_pokes[i]=&tabla_pokes[i];


        if (ret==1) {

                //cls_menu_overlay();
		int total=util_parse_pok_file(pokfile,punteros_pokes);

		if (total<1) {
			debug_printf (VERBOSE_ERR,"Error parsing POK file");
			return;
		}

		int j;
		for (j=0;j<total;j++) {
			debug_printf (VERBOSE_DEBUG,"menu poke index %d text %s bank %d address %d value %d value_orig %d",
				punteros_pokes[j]->indice_accion,
				punteros_pokes[j]->texto,
				punteros_pokes[j]->banco,
				punteros_pokes[j]->direccion,
				punteros_pokes[j]->valor,
				punteros_pokes[j]->valor_orig);
		}


		//Meter cada poke en un menu




        menu_item *array_menu_debug_pok_file;
        menu_item item_seleccionado;
        int retorno_menu;
	//Resetear siempre ultima linea = 0
	debug_pok_file_opcion_seleccionada=0;

	//temporal para mostrar todos los caracteres 0-255
	//int temp_conta=1;

        do {



		//Meter primer item de menu
		//truncar texto a 28 caracteres si excede de eso
		if (strlen(punteros_pokes[0]->texto)>28) punteros_pokes[0]->texto[28]=0;
                menu_add_item_menu_inicial_format(&array_menu_debug_pok_file,MENU_OPCION_NORMAL,NULL,NULL,"%s", punteros_pokes[0]->texto);


		//Luego recorrer array de pokes y cuando el numero de poke se incrementa, agregar
		int poke_anterior=0;

		int total_elementos=1;

		for (j=1;j<total;j++) {
			if (punteros_pokes[j]->indice_accion!=poke_anterior) {

				//temp para mostrar todos los caracteres 0-255
				//int kk;
				//for (kk=0;kk<strlen(punteros_pokes[j]->texto);kk++) {
				//	punteros_pokes[j]->texto[kk]=temp_conta++;
				//	if (temp_conta==256) temp_conta=1;
				//}

				poke_anterior=punteros_pokes[j]->indice_accion;
				//truncar texto a 28 caracteres si excede de eso
				if (strlen(punteros_pokes[j]->texto)>28) punteros_pokes[j]->texto[28]=0;
				menu_add_item_menu_format(array_menu_debug_pok_file,MENU_OPCION_NORMAL,NULL,NULL,"%s", punteros_pokes[j]->texto);

				total_elementos++;
				if (total_elementos==20) {
					debug_printf (VERBOSE_DEBUG,"Too many pokes to show on Window. Showing only first 20");
					menu_warn_message("Too many pokes to show on Window. Showing only first 20");
					break;
				}


			}
		}



                menu_add_item_menu(array_menu_debug_pok_file,"",MENU_OPCION_SEPARADOR,NULL,NULL);


                //menu_add_item_menu(array_menu_debug_pok_file,"ESC Back",MENU_OPCION_NORMAL|MENU_OPCION_ESC,NULL,NULL);
                menu_add_ESC_item(array_menu_debug_pok_file);

                retorno_menu=menu_dibuja_menu(&debug_pok_file_opcion_seleccionada,&item_seleccionado,array_menu_debug_pok_file,"Select Poke" );

                

                if ((item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu>=0) {
                        //llamamos por valor de funcion

			//Hacer poke sabiendo la linea seleccionada. Desde ahi, ejecutar todos los pokes de dicha accion
			debug_printf (VERBOSE_DEBUG,"Doing poke/s from line %d",debug_pok_file_opcion_seleccionada);

			z80_byte banco;
			z80_int direccion;
			z80_byte valor;

			//buscar indice_accion
			int result_poke=0;
			for (j=0;j<total && result_poke==0;j++) {

				debug_printf (VERBOSE_DEBUG,"index %d looking %d current %d",j,debug_pok_file_opcion_seleccionada,punteros_pokes[j]->indice_accion);

				if (punteros_pokes[j]->indice_accion==debug_pok_file_opcion_seleccionada) {
					banco=punteros_pokes[j]->banco;
					direccion=punteros_pokes[j]->direccion;
					valor=punteros_pokes[j]->valor;
					debug_printf (VERBOSE_DEBUG,"Doing poke bank %d address %d value %d",banco,direccion,valor);
					result_poke=util_poke(banco,direccion,valor);
				}


                      

			}
			if (result_poke==0) menu_generic_message("Poke","OK. Poke applied");
                }

        } while ( (item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu!=MENU_RETORNO_ESC && !salir_todos_menus);




        }

}





void menu_poke(MENU_ITEM_PARAMETERS)
{
        menu_item *array_menu_poke;
        menu_item item_seleccionado;
        int retorno_menu;

        do {


                menu_add_item_menu_inicial_format(&array_menu_poke,MENU_OPCION_NORMAL,menu_debug_poke,NULL,"~~Poke");
                menu_add_item_menu_shortcut(array_menu_poke,'p');
                menu_add_item_menu_tooltip(array_menu_poke,"Poke address");
                menu_add_item_menu_ayuda(array_menu_poke,"Poke address for infinite lives, etc... This item follows active memory zone. "
					"You can also poke on read-only memory, depending on the current memory zone");

		//No tiene sentido pues se puede usar las memory zones para esto
		/*if (MACHINE_IS_SPECTRUM_128_P2_P2A_P3 || MACHINE_IS_ZXUNO_BOOTM_DISABLED) {
			menu_add_item_menu(array_menu_poke,"Poke 128~~k mode",MENU_OPCION_NORMAL,menu_debug_poke_128k,NULL);
			menu_add_item_menu_shortcut(array_menu_poke,'k');
			menu_add_item_menu_tooltip(array_menu_poke,"Poke bank & address");
			menu_add_item_menu_ayuda(array_menu_poke,"Poke bank & address");
		}*/

		if (MACHINE_IS_SPECTRUM) {
			menu_add_item_menu(array_menu_poke,"Poke from .POK ~~file",MENU_OPCION_NORMAL,menu_debug_poke_pok_file,NULL);
			menu_add_item_menu_shortcut(array_menu_poke,'f');
			menu_add_item_menu_tooltip(array_menu_poke,"Poke reading .POK file");
			menu_add_item_menu_ayuda(array_menu_poke,"Poke reading .POK file");
		}


                menu_add_item_menu(array_menu_poke,"",MENU_OPCION_SEPARADOR,NULL,NULL);


                //menu_add_item_menu(array_menu_poke,"ESC Back",MENU_OPCION_NORMAL|MENU_OPCION_ESC,NULL,NULL);
                menu_add_ESC_item(array_menu_poke);

                retorno_menu=menu_dibuja_menu(&poke_opcion_seleccionada,&item_seleccionado,array_menu_poke,"Poke" );

                

                if ((item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu>=0) {
                        //llamamos por valor de funcion
                        if (item_seleccionado.menu_funcion!=NULL) {
                                //printf ("actuamos por funcion\n");
                                item_seleccionado.menu_funcion(item_seleccionado.valor_opcion);
                                
                        }
                }

        } while ( (item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu!=MENU_RETORNO_ESC && !salir_todos_menus);
}


void menu_debug_registers_console(MENU_ITEM_PARAMETERS) {
	debug_registers ^=1;
}

void menu_debug_configuration_stepover(MENU_ITEM_PARAMETERS)
{
	//debug_core_evitamos_inter.v ^=1;
	remote_debug_settings ^=32;
}


void menu_breakpoints_condition_behaviour(MENU_ITEM_PARAMETERS)
{
	debug_breakpoints_cond_behaviour.v ^=1;
}

void menu_debug_configuration_remoteproto_port(MENU_ITEM_PARAMETERS)
{
	char string_port[6];
	int port;

	sprintf (string_port,"%d",remote_protocol_port);

	menu_ventana_scanf("Port",string_port,6);

	if (string_port[0]==0) return;

	else {
			port=parse_string_to_number(string_port);

			if (port<1 || port>65535) {
								debug_printf (VERBOSE_ERR,"Invalid port %d",port);
								return;
			}


			end_remote_protocol();
			remote_protocol_port=port;
			init_remote_protocol();
	}

}

void menu_debug_shows_invalid_opcode(MENU_ITEM_PARAMETERS)
{
	debug_shows_invalid_opcode.v ^=1;
}

void menu_debug_settings_show_fired_breakpoint(MENU_ITEM_PARAMETERS)
{
	debug_show_fired_breakpoints_type++;
	if (debug_show_fired_breakpoints_type==3) debug_show_fired_breakpoints_type=0;
}

void menu_debug_settings_show_screen(MENU_ITEM_PARAMETERS)
{
	debug_settings_show_screen.v ^=1;
}
void menu_debug_settings_show_scanline(MENU_ITEM_PARAMETERS)
{
	menu_debug_registers_if_showscan.v ^=1;
}

void menu_debug_configuration_remoteproto(MENU_ITEM_PARAMETERS)
{
	if (remote_protocol_enabled.v) {
		end_remote_protocol();
		remote_protocol_enabled.v=0;
	}

	else {
		remote_protocol_enabled.v=1;
		init_remote_protocol();
	}
}


void menu_debug_verbose(MENU_ITEM_PARAMETERS)
{
	verbose_level++;
	if (verbose_level>4) verbose_level=0;
}

void menu_zesarux_zxi_hardware_debug_file(MENU_ITEM_PARAMETERS)
{

	char *filtros[2];

    filtros[0]="";
    filtros[1]=0;


    if (menu_filesel("Select Debug File",filtros,zesarux_zxi_hardware_debug_file)==1) {
    	//Ver si archivo existe y preguntar
		if (si_existe_archivo(zesarux_zxi_hardware_debug_file)) {
            if (menu_confirm_yesno_texto("File exists","Append?")==0) {
				zesarux_zxi_hardware_debug_file[0]=0;
				return;
			}
        }

    }

	else zesarux_zxi_hardware_debug_file[0]=0;

}

void menu_hardware_debug_port(MENU_ITEM_PARAMETERS)
{
	hardware_debug_port.v ^=1;
}

//menu debug settings
void menu_settings_debug(MENU_ITEM_PARAMETERS)
{
        menu_item *array_menu_settings_debug;
        menu_item item_seleccionado;
	int retorno_menu;
        do {


      char string_zesarux_zxi_hardware_debug_file_shown[18];
      


		menu_add_item_menu_inicial_format(&array_menu_settings_debug,MENU_OPCION_NORMAL,menu_debug_registers_console,NULL,"[%c] Show r~~egisters in console",(debug_registers==1 ? 'X' : ' '));
		menu_add_item_menu_shortcut(array_menu_settings_debug,'e');

		menu_add_item_menu_format(array_menu_settings_debug,MENU_OPCION_NORMAL,menu_debug_shows_invalid_opcode,NULL,"[%c] Show ~~invalid opcode",
			(debug_shows_invalid_opcode.v ? 'X' : ' ') ); 
		menu_add_item_menu_shortcut(array_menu_settings_debug,'i');
		menu_add_item_menu_tooltip(array_menu_settings_debug,"Show which opcodes are invalid (considering ED, DD, FD prefixes)");
		menu_add_item_menu_ayuda(array_menu_settings_debug,"Show which opcodes are invalid (considering ED, DD, FD prefixes). "
								"A message will be shown on console, when verbose level is 2 or higher");





        


		menu_add_item_menu_format(array_menu_settings_debug,MENU_OPCION_NORMAL,menu_debug_configuration_stepover,NULL,"[%c] Step ~~over interrupt",(remote_debug_settings&32 ? 'X' : ' ') );
		menu_add_item_menu_tooltip(array_menu_settings_debug,"Avoid step to step or continuous execution of nmi or maskable interrupt routines on debug cpu menu");
		menu_add_item_menu_ayuda(array_menu_settings_debug,"Avoid step to step or continuous execution of nmi or maskable interrupt routines on debug cpu menu");
		menu_add_item_menu_shortcut(array_menu_settings_debug,'o');









		menu_add_item_menu_format(array_menu_settings_debug,MENU_OPCION_NORMAL, menu_debug_settings_show_screen,NULL,"[%c] Show display on debug",
			( debug_settings_show_screen.v ? 'X' : ' ') );
		menu_add_item_menu_tooltip(array_menu_settings_debug,"If shows emulated screen on every key action on debug registers menu");	
		menu_add_item_menu_ayuda(array_menu_settings_debug,"If shows emulated screen on every key action on debug registers menu");	


		menu_add_item_menu_format(array_menu_settings_debug,MENU_OPCION_NORMAL, menu_debug_settings_show_scanline,NULL,"[%c] Shows electron on debug",
			( menu_debug_registers_if_showscan.v ? 'X' : ' ') );
		menu_add_item_menu_tooltip(array_menu_settings_debug,"Shows TV electron position when debugging, using a coloured line. Requires real video");
		menu_add_item_menu_ayuda(array_menu_settings_debug,"Shows TV electron position when debugging, using a coloured line. Requires real video");


		menu_add_item_menu_format(array_menu_settings_debug,MENU_OPCION_NORMAL,menu_debug_verbose,NULL,"[%d] Verbose ~~level",verbose_level);
		menu_add_item_menu_shortcut(array_menu_settings_debug,'l');		


					




		menu_add_item_menu(array_menu_settings_debug,"",MENU_OPCION_SEPARADOR,NULL,NULL);		


#ifdef USE_PTHREADS
		menu_add_item_menu_format(array_menu_settings_debug,MENU_OPCION_NORMAL, menu_debug_configuration_remoteproto,NULL,"[%c] ZRCP Remote protocol",(remote_protocol_enabled.v ? 'X' : ' ') );
		menu_add_item_menu_tooltip(array_menu_settings_debug,"Enables or disables ZEsarUX remote command protocol (ZRCP)");
		menu_add_item_menu_ayuda(array_menu_settings_debug,"Enables or disables ZEsarUX remote command protocol (ZRCP)");
		menu_add_item_menu_shortcut(array_menu_settings_debug,'r');

		if (remote_protocol_enabled.v) {
			menu_add_item_menu_format(array_menu_settings_debug,MENU_OPCION_NORMAL, menu_debug_configuration_remoteproto_port,NULL,"[%d] Remote protocol ~~port",remote_protocol_port );
			menu_add_item_menu_tooltip(array_menu_settings_debug,"Changes remote command protocol port");
			menu_add_item_menu_ayuda(array_menu_settings_debug,"Changes remote command protocol port");
			menu_add_item_menu_shortcut(array_menu_settings_debug,'p');
		}

#endif


		menu_add_item_menu_format(array_menu_settings_debug,MENU_OPCION_NORMAL, menu_hardware_debug_port,NULL,"[%c] Hardware ~~debug ports",(hardware_debug_port.v ? 'X' : ' ') );
		menu_add_item_menu_tooltip(array_menu_settings_debug,"If hardware debug ports are enabled");
		menu_add_item_menu_ayuda(array_menu_settings_debug,"These ports are used to interact with ZEsarUX, for example showing a ASCII character on console, read ZEsarUX version, etc. "
														"Read file extras/docs/zesarux_zxi_registers.txt for more information");
		menu_add_item_menu_shortcut(array_menu_settings_debug,'d');


		if (hardware_debug_port.v) {
			menu_tape_settings_trunc_name(zesarux_zxi_hardware_debug_file,string_zesarux_zxi_hardware_debug_file_shown,18);
        	menu_add_item_menu_format(array_menu_settings_debug,MENU_OPCION_NORMAL,menu_zesarux_zxi_hardware_debug_file,NULL,"Byte ~~file [%s]",string_zesarux_zxi_hardware_debug_file_shown);
			menu_add_item_menu_tooltip(array_menu_settings_debug,"File used on using register 6 (HARDWARE_DEBUG_BYTE_FILE)");		
			menu_add_item_menu_ayuda(array_menu_settings_debug,"File used on using register 6 (HARDWARE_DEBUG_BYTE_FILE)");	
			menu_add_item_menu_shortcut(array_menu_settings_debug,'f');							
		}


		menu_add_item_menu(array_menu_settings_debug,"",MENU_OPCION_SEPARADOR,NULL,NULL);

		

		menu_add_item_menu_format(array_menu_settings_debug,MENU_OPCION_NORMAL, menu_breakpoints_condition_behaviour,NULL,"~~Breakp. behaviour [%s]",(debug_breakpoints_cond_behaviour.v ? "On Change" : "Always") );
		menu_add_item_menu_tooltip(array_menu_settings_debug,"Indicates whether breakpoints are fired always or only on change from false to true");
		menu_add_item_menu_ayuda(array_menu_settings_debug,"Indicates whether breakpoints are fired always or only on change from false to true");
		menu_add_item_menu_shortcut(array_menu_settings_debug,'b');


		char show_fired_breakpoint_type[30];
		if (debug_show_fired_breakpoints_type==0) strcpy(show_fired_breakpoint_type,"Always");
		else if (debug_show_fired_breakpoints_type==1) strcpy(show_fired_breakpoint_type,"NoPC");
		else strcpy(show_fired_breakpoint_type,"Never");																	//						   OnlyNonPC
																															//  01234567890123456789012345678901
		menu_add_item_menu_format(array_menu_settings_debug,MENU_OPCION_NORMAL, menu_debug_settings_show_fired_breakpoint,NULL,"Show fired breakpoint [%s]",show_fired_breakpoint_type);
		menu_add_item_menu_tooltip(array_menu_settings_debug,"Tells to show the breakpoint condition when it is fired");
		menu_add_item_menu_ayuda(array_menu_settings_debug,"Tells to show the breakpoint condition when it is fired. "
								"Possible values:\n"
								"Always: always shows the condition\n"
								"NoPC: only shows conditions that are not like PC=XXXX\n"
								"Never: never shows conditions\n" );			


                menu_add_item_menu(array_menu_settings_debug,"",MENU_OPCION_SEPARADOR,NULL,NULL);
                //menu_add_item_menu(array_menu_settings_debug,"ESC Back",MENU_OPCION_NORMAL|MENU_OPCION_ESC,NULL,NULL);
		menu_add_ESC_item(array_menu_settings_debug);

                retorno_menu=menu_dibuja_menu(&settings_debug_opcion_seleccionada,&item_seleccionado,array_menu_settings_debug,"Debug Settings" );

                

		if ((item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu>=0) {
                        //llamamos por valor de funcion
                        if (item_seleccionado.menu_funcion!=NULL) {
                                //printf ("actuamos por funcion\n");
                                item_seleccionado.menu_funcion(item_seleccionado.valor_opcion);
				
                        }
                }

	} while ( (item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu!=MENU_RETORNO_ESC && !salir_todos_menus);

}



int num_menu_audio_driver;
int num_previo_menu_audio_driver;


//Determina cual es el audio driver actual
void menu_change_audio_driver_get(void)
{
        int i;
        for (i=0;i<num_audio_driver_array;i++) {
		//printf ("actual: %s buscado: %s indice: %d\n",audio_driver_name,audio_driver_array[i].driver_name,i);
                if (!strcmp(audio_driver_name,audio_driver_array[i].driver_name)) {
                        num_menu_audio_driver=i;
                        num_previo_menu_audio_driver=i;
			return;
                }

        }

}


void menu_change_audio_driver_change(MENU_ITEM_PARAMETERS)
{
        num_menu_audio_driver++;
        if (num_menu_audio_driver==num_audio_driver_array) num_menu_audio_driver=0;
}

void menu_change_audio_driver_apply(MENU_ITEM_PARAMETERS)
{

	audio_end();

        int (*funcion_init) ();
        int (*funcion_set) ();

        funcion_init=audio_driver_array[num_menu_audio_driver].funcion_init;
        funcion_set=audio_driver_array[num_menu_audio_driver].funcion_set;
                if ( (funcion_init()) ==0) {
                        funcion_set();
			menu_generic_message("Apply Driver","OK. Driver applied");
			salir_todos_menus=1;
                }

                else {
                        debug_printf(VERBOSE_ERR,"Can not set audio driver. Restoring to previous driver %s",audio_driver_name);
			menu_change_audio_driver_get();

                        //Restaurar audio driver
                        funcion_init=audio_driver_array[num_previo_menu_audio_driver].funcion_init;
                        funcion_set=audio_driver_array[num_previo_menu_audio_driver].funcion_set;

                        funcion_init();
                        funcion_set();
                }



}


void menu_change_audio_driver(MENU_ITEM_PARAMETERS)
{
        menu_item *array_menu_change_audio_driver;
        menu_item item_seleccionado;
        int retorno_menu;

       	menu_change_audio_driver_get();

        do {

                menu_add_item_menu_inicial_format(&array_menu_change_audio_driver,MENU_OPCION_NORMAL,menu_change_audio_driver_change,NULL,"Audio Driver: %s",audio_driver_array[num_menu_audio_driver].driver_name );

                menu_add_item_menu_format(array_menu_change_audio_driver,MENU_OPCION_NORMAL,menu_change_audio_driver_apply,NULL,"Apply Driver" );

                menu_add_item_menu(array_menu_change_audio_driver,"",MENU_OPCION_SEPARADOR,NULL,NULL);
                //menu_add_item_menu(array_menu_change_audio_driver,"ESC Back",MENU_OPCION_NORMAL|MENU_OPCION_ESC,NULL,NULL);
                menu_add_ESC_item(array_menu_change_audio_driver);

                retorno_menu=menu_dibuja_menu(&change_audio_driver_opcion_seleccionada,&item_seleccionado,array_menu_change_audio_driver,"Change Audio Driver" );

                

                if ((item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu>=0) {
                        //llamamos por valor de funcion
                        if (item_seleccionado.menu_funcion!=NULL) {
                                //printf ("actuamos por funcion\n");
                                item_seleccionado.menu_funcion(item_seleccionado.valor_opcion);
                                
                        }
                }

        } while ( (item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu!=MENU_RETORNO_ESC && !salir_todos_menus);
}






int menu_cond_ay_chip(void)
{
	return ay_chip_present.v;
}


void menu_audio_beep_filter_on_rom_save(MENU_ITEM_PARAMETERS)
{
	output_beep_filter_on_rom_save.v ^=1;
}


void menu_audio_beep_alter_volume(MENU_ITEM_PARAMETERS)
{
	output_beep_filter_alter_volume.v ^=1;
}


void menu_audio_beep_volume(MENU_ITEM_PARAMETERS)
{

        char string_vol[4];

        sprintf (string_vol,"%d",output_beep_filter_volume);


        menu_ventana_scanf("Volume (0-127)",string_vol,4);

        int v=parse_string_to_number(string_vol);

        if (v>127 || v<0) {
                debug_printf (VERBOSE_ERR,"Invalid volume value");
                return;
        }

        output_beep_filter_volume=v;
}

void menu_audio_beeper_real (MENU_ITEM_PARAMETERS)
{
	beeper_real_enabled ^=1;
}

void menu_audio_volume(MENU_ITEM_PARAMETERS)
{
        char string_perc[4];

        sprintf (string_perc,"%d",audiovolume);


        menu_ventana_scanf("Volume in %",string_perc,4);

        int v=parse_string_to_number(string_perc);

	if (v>100 || v<0) {
		debug_printf (VERBOSE_ERR,"Invalid volume value");
		return;
	}

	audiovolume=v;
}

void menu_audio_ay_chip(MENU_ITEM_PARAMETERS)
{
	ay_chip_present.v^=1;
}

void menu_audio_ay_chip_autoenable(MENU_ITEM_PARAMETERS)
{
	autoenable_ay_chip.v^=1;
}

void menu_audio_envelopes(MENU_ITEM_PARAMETERS)
{
	ay_envelopes_enabled.v^=1;
}

void menu_audio_speech(MENU_ITEM_PARAMETERS)
{
        ay_speech_enabled.v^=1;
}

void menu_audio_sound_zx8081(MENU_ITEM_PARAMETERS)
{
	zx8081_vsync_sound.v^=1;
}

void menu_audio_zx8081_detect_vsync_sound(MENU_ITEM_PARAMETERS)
{
	zx8081_detect_vsync_sound.v ^=1;
}



void menu_setting_ay_piano_grafico(MENU_ITEM_PARAMETERS)
{
	setting_mostrar_ay_piano_grafico.v ^=1;
}


void menu_aofile_insert(MENU_ITEM_PARAMETERS)
{

	if (aofile_inserted.v==0) {
		init_aofile();

		//Si todo ha ido bien
		if (aofile_inserted.v) {
			menu_generic_message_format("File information","%s\n%s\n\n%s",
			last_message_helper_aofile_vofile_file_format,last_message_helper_aofile_vofile_bytes_minute_audio,last_message_helper_aofile_vofile_util);
		}

	}

        else if (aofile_inserted.v==1) {
                close_aofile();
        }

}

int menu_aofile_cond(void)
{
	if (aofilename!=NULL) return 1;
	else return 0;
}

void menu_aofile(MENU_ITEM_PARAMETERS)
{

	aofile_inserted.v=0;


        char *filtros[3];

#ifdef USE_SNDFILE
        filtros[0]="rwa";
        filtros[1]="wav";
        filtros[2]=0;
#else
        filtros[0]="rwa";
        filtros[1]=0;
#endif


        if (menu_filesel("Select Audio File",filtros,aofilename_file)==1) {

       	        if (si_existe_archivo(aofilename_file)) {

               	        if (menu_confirm_yesno_texto("File exists","Overwrite?")==0) {
				aofilename=NULL;
				return;
			}

       	        }

                aofilename=aofilename_file;


        }

	else {
		aofilename=NULL;
	}


}




/*void menu_audio_audiodac(MENU_ITEM_PARAMETERS)
{
	audiodac_enabled.v ^=1;
}*/

void menu_audio_audiodac_type(MENU_ITEM_PARAMETERS)
{
	if (audiodac_enabled.v==0) {
		audiodac_enabled.v=1;
		audiodac_selected_type=0;
	}

	else {
		audiodac_selected_type++;
		if (audiodac_selected_type==MAX_AUDIODAC_TYPES) {
			audiodac_selected_type=0;
			audiodac_enabled.v=0;
		}
	}
}

void menu_audio_audiodac_set_port(MENU_ITEM_PARAMETERS)
{
	char string_port[4];

	sprintf (string_port,"%02XH",audiodac_types[MAX_AUDIODAC_TYPES-1].port);

	menu_ventana_scanf("Port Value",string_port,4);

	int valor_port=parse_string_to_number(string_port);

	if (valor_port<0 || valor_port>255) {
					debug_printf (VERBOSE_ERR,"Invalid value %d",valor_port);
					return;
	}

	audiodac_set_custom_port(valor_port);
	//audiodac_types[MAX_AUDIODAC_TYPES-1].port=valor_port;
	//audiodac_selected_type=MAX_AUDIODAC_TYPES-1;

}

void menu_audio_beeper(MENU_ITEM_PARAMETERS)
{
	beeper_enabled.v ^=1;
}

void menu_audio_change_ay_chips(MENU_ITEM_PARAMETERS)
{
	if (total_ay_chips==MAX_AY_CHIPS) total_ay_chips=1;
	else total_ay_chips++;

	ay_chip_selected=0;
}

void menu_audio_ay_stereo(MENU_ITEM_PARAMETERS)
{
	ay3_stereo_mode++;

	if (ay3_stereo_mode==5) ay3_stereo_mode=0;
}


void menu_audio_ay_stereo_custom(MENU_ITEM_PARAMETERS)
{
	ay3_custom_stereo_A++;
	if (ay3_custom_stereo_A==3) {
		ay3_custom_stereo_A=2;

		ay3_custom_stereo_B++;
		if (ay3_custom_stereo_B==3) {
			ay3_custom_stereo_B=2;

			ay3_custom_stereo_C++;
			if (ay3_custom_stereo_C==3) {
				ay3_custom_stereo_A=0;
				ay3_custom_stereo_B=0;
				ay3_custom_stereo_C=0;
			}
		}
	}	
}

void menu_audio_ay_stereo_custom_A(MENU_ITEM_PARAMETERS)
{
	ay3_custom_stereo_A++;
	if (ay3_custom_stereo_A==3) ay3_custom_stereo_A=0;
}

void menu_audio_ay_stereo_custom_B(MENU_ITEM_PARAMETERS)
{
	ay3_custom_stereo_B++;
	if (ay3_custom_stereo_B==3) ay3_custom_stereo_B=0;
}

void menu_audio_ay_stereo_custom_C(MENU_ITEM_PARAMETERS)
{
	ay3_custom_stereo_C++;
	if (ay3_custom_stereo_C==3) ay3_custom_stereo_C=0;
}

char *menu_stereo_positions[]={
	"Left",
	"    Center",
	"          Right"
};


void menu_silence_detector(MENU_ITEM_PARAMETERS)
{
	silence_detector_setting.v ^=1;
}

void menu_settings_audio(MENU_ITEM_PARAMETERS)
{
        menu_item *array_menu_settings_audio;
	menu_item item_seleccionado;
	int retorno_menu;

        do {

		//hotkeys usadas: vapesboiuctdrf

		menu_add_item_menu_inicial_format(&array_menu_settings_audio,MENU_OPCION_NORMAL,menu_audio_volume,NULL,"    Output ~~Volume [%d %%]", audiovolume);
		menu_add_item_menu_shortcut(array_menu_settings_audio,'v');

		menu_add_item_menu_format(array_menu_settings_audio,MENU_OPCION_NORMAL,menu_audio_ay_chip_autoenable,NULL,"[%c] A~~utoenable AY Chip",(autoenable_ay_chip.v==1 ? 'X' : ' '));
		menu_add_item_menu_shortcut(array_menu_settings_audio,'u');
		menu_add_item_menu_tooltip(array_menu_settings_audio,"Enable AY Chip automatically when it is needed");
		menu_add_item_menu_ayuda(array_menu_settings_audio,"This option is usefor for example on Spectrum 48k games that uses AY Chip "
					"and for some ZX80/81 games that also uses it (Bi-Pak ZON-X81, but not Quicksilva QS Sound board)");		

		menu_add_item_menu_format(array_menu_settings_audio,MENU_OPCION_NORMAL,menu_audio_ay_chip,NULL,"[%c] ~~AY Chip", (ay_chip_present.v==1 ? 'X' : ' '));
		menu_add_item_menu_shortcut(array_menu_settings_audio,'a');
		menu_add_item_menu_tooltip(array_menu_settings_audio,"Enable AY Chip on this machine");
		menu_add_item_menu_ayuda(array_menu_settings_audio,"It enables the AY Chip for the machine, by activating the following hardware:\n"
					"-Normal AY Chip for Spectrum\n"
					"-Fuller audio box for Spectrum\n"
					"-Quicksilva QS Sound board on ZX80/81\n"
					"-Bi-Pak ZON-X81 Sound on ZX80/81\n"
			);



			menu_add_item_menu_format(array_menu_settings_audio,MENU_OPCION_NORMAL,menu_audio_change_ay_chips,menu_cond_ay_chip,"[%d] AY ~~Chips %s",total_ay_chips,
				(total_ay_chips>1 ? "(Turbosound)" : "") );
			menu_add_item_menu_shortcut(array_menu_settings_audio,'c');

		if (si_complete_video_driver() ) {
			menu_add_item_menu_format(array_menu_settings_audio,MENU_OPCION_NORMAL,menu_setting_ay_piano_grafico,NULL,"    Show ~~Piano: %s",
					(setting_mostrar_ay_piano_grafico.v ? "Graphic" : "Text") );
			menu_add_item_menu_shortcut(array_menu_settings_audio,'p');
			menu_add_item_menu_tooltip(array_menu_settings_audio,"Shows AY/Beeper Piano menu with graphic or with text");
			menu_add_item_menu_ayuda(array_menu_settings_audio,"Shows AY/Beeper Piano menu with graphic or with text");

		}


		menu_add_item_menu_format(array_menu_settings_audio,MENU_OPCION_NORMAL,menu_audio_envelopes,menu_cond_ay_chip,"[%c] AY ~~Envelopes", (ay_envelopes_enabled.v==1 ? 'X' : ' '));
		menu_add_item_menu_shortcut(array_menu_settings_audio,'e');
		menu_add_item_menu_tooltip(array_menu_settings_audio,"Enable or disable volume envelopes for the AY Chip");
		menu_add_item_menu_ayuda(array_menu_settings_audio,"Enable or disable volume envelopes for the AY Chip");

		menu_add_item_menu_format(array_menu_settings_audio,MENU_OPCION_NORMAL,menu_audio_speech,menu_cond_ay_chip,"[%c] AY ~~Speech", (ay_speech_enabled.v==1 ? 'X' : ' '));
		menu_add_item_menu_shortcut(array_menu_settings_audio,'s');
		menu_add_item_menu_tooltip(array_menu_settings_audio,"Enable or disable AY Speech effects");
		menu_add_item_menu_ayuda(array_menu_settings_audio,"These effects are used, for example, in Chase H.Q.");


		if (MACHINE_IS_SPECTRUM) {


			char ay3_stereo_string[16];
			if (ay3_stereo_mode==1) strcpy(ay3_stereo_string,"ACB");
			else if (ay3_stereo_mode==2) strcpy(ay3_stereo_string,"ABC");
			else if (ay3_stereo_mode==3) strcpy(ay3_stereo_string,"BAC");
			else if (ay3_stereo_mode==4) strcpy(ay3_stereo_string,"Custom");
			else strcpy(ay3_stereo_string,"Mono");

			menu_add_item_menu_format(array_menu_settings_audio,MENU_OPCION_NORMAL,menu_audio_ay_stereo,menu_cond_ay_chip,"    AY S~~tereo: %s",
				ay3_stereo_string);
			menu_add_item_menu_shortcut(array_menu_settings_audio,'t');

			if (ay3_stereo_mode==4) {	

				menu_add_item_menu_format(array_menu_settings_audio,MENU_OPCION_NORMAL,menu_audio_ay_stereo_custom_A,menu_cond_ay_chip,
					"    Ch. A: %s",menu_stereo_positions[ay3_custom_stereo_A]);

				menu_add_item_menu_format(array_menu_settings_audio,MENU_OPCION_NORMAL,menu_audio_ay_stereo_custom_B,menu_cond_ay_chip,
					"    Ch. B: %s",menu_stereo_positions[ay3_custom_stereo_B]);

				menu_add_item_menu_format(array_menu_settings_audio,MENU_OPCION_NORMAL,menu_audio_ay_stereo_custom_C,menu_cond_ay_chip,
					"    Ch. C: %s",menu_stereo_positions[ay3_custom_stereo_C]);								


			}

		}



		if (MACHINE_IS_SPECTRUM) {

			menu_add_item_menu(array_menu_settings_audio,"",MENU_OPCION_SEPARADOR,NULL,NULL);

			char string_audiodac[32];

				if (audiodac_enabled.v) {
					sprintf (string_audiodac,": %s",audiodac_types[audiodac_selected_type].name);
				}
				else {
					strcpy(string_audiodac,"");
				}

				menu_add_item_menu_format(array_menu_settings_audio,MENU_OPCION_NORMAL,menu_audio_audiodac_type,NULL,"[%c] ~~DAC%s",(audiodac_enabled.v ? 'X' : ' ' ),
						string_audiodac);
				menu_add_item_menu_shortcut(array_menu_settings_audio,'d');
				if (audiodac_enabled.v) {
					menu_add_item_menu_format(array_menu_settings_audio,MENU_OPCION_NORMAL,menu_audio_audiodac_set_port,NULL,"[%02XH] DAC port",audiodac_types[audiodac_selected_type].port);
				}



		}


    menu_add_item_menu(array_menu_settings_audio,"",MENU_OPCION_SEPARADOR,NULL,NULL);


		if (!MACHINE_IS_ZX8081) {

			menu_add_item_menu_format(array_menu_settings_audio,MENU_OPCION_NORMAL,menu_audio_beeper,NULL,"[%c] Beepe~~r",(beeper_enabled.v==1 ? 'X' : ' '));
			menu_add_item_menu_shortcut(array_menu_settings_audio,'r');
			menu_add_item_menu_tooltip(array_menu_settings_audio,"Enable or disable beeper output");
			menu_add_item_menu_ayuda(array_menu_settings_audio,"Enable or disable beeper output");

		}



		if (MACHINE_IS_ZX8081) {
			//sound on zx80/81

			menu_add_item_menu_format(array_menu_settings_audio,MENU_OPCION_NORMAL,menu_audio_zx8081_detect_vsync_sound,menu_cond_zx8081,"[%c] Detect VSYNC Sound",(zx8081_detect_vsync_sound.v ? 'X' : ' '));
			menu_add_item_menu_tooltip(array_menu_settings_audio,"Tries to detect when vsync sound is played. This feature is experimental");
			menu_add_item_menu_ayuda(array_menu_settings_audio,"Tries to detect when vsync sound is played. This feature is experimental");


			menu_add_item_menu_format(array_menu_settings_audio,MENU_OPCION_NORMAL,menu_audio_sound_zx8081,menu_cond_zx8081,"[%c] VSYNC Sound on zx80/81", (zx8081_vsync_sound.v==1 ? 'X' : ' '));
			menu_add_item_menu_tooltip(array_menu_settings_audio,"Enables or disables VSYNC sound on ZX80 and ZX81");
			menu_add_item_menu_ayuda(array_menu_settings_audio,"This method uses the VSYNC signal on the TV to make sound");


		}




		int mostrar_real_beeper=0;

		if (MACHINE_IS_ZX8081) {
			if (zx8081_vsync_sound.v) mostrar_real_beeper=1;
		}

		else {
			if (beeper_enabled.v) mostrar_real_beeper=1;
		}

		if (mostrar_real_beeper) {

			menu_add_item_menu_format(array_menu_settings_audio,MENU_OPCION_NORMAL,menu_audio_beeper_real,NULL,"[%c] Real ~~Beeper",(beeper_real_enabled==1 ? 'X' : ' '));
			menu_add_item_menu_shortcut(array_menu_settings_audio,'b');
			menu_add_item_menu_tooltip(array_menu_settings_audio,"Enable or disable Real Beeper enhanced sound. ");
			menu_add_item_menu_ayuda(array_menu_settings_audio,"Real beeper produces beeper sound more realistic but uses a bit more cpu. Needs beeper enabled (or vsync sound on zx80/81)");
		}


		if (MACHINE_IS_SPECTRUM) {
			menu_add_item_menu_format(array_menu_settings_audio,MENU_OPCION_NORMAL,menu_audio_beep_filter_on_rom_save,NULL,"[%c] Audio ~~filter ROM SAVE",(output_beep_filter_on_rom_save.v ? 'X' : ' '));
			menu_add_item_menu_shortcut(array_menu_settings_audio,'f');
			menu_add_item_menu_tooltip(array_menu_settings_audio,"Apply filter on ROM save routines");
			menu_add_item_menu_ayuda(array_menu_settings_audio,"It detects when on ROM save routines and alter audio output to use only "
					"the MIC bit of the FEH port");

//extern z80_bit output_beep_filter_alter_volume;
//extern char output_beep_filter_volume;

			if (output_beep_filter_on_rom_save.v) {
				menu_add_item_menu_format(array_menu_settings_audio,MENU_OPCION_NORMAL,menu_audio_beep_alter_volume,NULL,"[%c] Alter beeper volume",
				(output_beep_filter_alter_volume.v ? 'X' : ' ') );

				menu_add_item_menu_tooltip(array_menu_settings_audio,"Alter output beeper volume");
				menu_add_item_menu_ayuda(array_menu_settings_audio,"Alter output beeper volume. You can set to a maximum to "
							"send the audio to a real spectrum to load it");


				if (output_beep_filter_alter_volume.v) {
					menu_add_item_menu_format(array_menu_settings_audio,MENU_OPCION_NORMAL,menu_audio_beep_volume,NULL,"[%d] Volume",output_beep_filter_volume);
				}
			}

		}

	
		menu_add_item_menu(array_menu_settings_audio,"",MENU_OPCION_SEPARADOR,NULL,NULL);


		char string_aofile_shown[10];
		menu_tape_settings_trunc_name(aofilename,string_aofile_shown,10);
		menu_add_item_menu_format(array_menu_settings_audio,MENU_OPCION_NORMAL,menu_aofile,NULL,"Audio ~~out to file [%s]",string_aofile_shown);
		menu_add_item_menu_shortcut(array_menu_settings_audio,'o');
		menu_add_item_menu_tooltip(array_menu_settings_audio,"Saves the generated sound to a file");
		menu_add_item_menu_ayuda(array_menu_settings_audio,"You can save .raw format and if compiled with sndfile, to .wav format. "
					"You can see the file parameters on the console enabling verbose debug level to 2 minimum");



		menu_add_item_menu_format(array_menu_settings_audio,MENU_OPCION_NORMAL,menu_aofile_insert,menu_aofile_cond,"[%c] Audio file ~~inserted",(aofile_inserted.v ? 'X' : ' ' ));
		menu_add_item_menu_shortcut(array_menu_settings_audio,'i');


				menu_add_item_menu_format(array_menu_settings_audio,MENU_OPCION_NORMAL,menu_silence_detector,NULL,"[%c] Si~~lence detector",(silence_detector_setting.v ? 'X' : ' ' ));
				menu_add_item_menu_shortcut(array_menu_settings_audio,'l');
				menu_add_item_menu_tooltip(array_menu_settings_audio,"Change this setting if you are listening some audio 'clicks'");
				menu_add_item_menu_ayuda(array_menu_settings_audio,"Change this setting if you are listening some audio 'clicks'");

                menu_add_item_menu_format(array_menu_settings_audio,MENU_OPCION_NORMAL,menu_change_audio_driver,NULL,"    C~~hange Audio Driver");
				menu_add_item_menu_shortcut(array_menu_settings_audio,'h');



                menu_add_item_menu(array_menu_settings_audio,"",MENU_OPCION_SEPARADOR,NULL,NULL);

		menu_add_ESC_item(array_menu_settings_audio);

                retorno_menu=menu_dibuja_menu(&settings_audio_opcion_seleccionada,&item_seleccionado,array_menu_settings_audio,"Audio Settings" );

                

		if ((item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu>=0) {
	                //llamamos por valor de funcion
        	        if (item_seleccionado.menu_funcion!=NULL) {
                	        //printf ("actuamos por funcion\n");
	                        item_seleccionado.menu_funcion(item_seleccionado.valor_opcion);
				
        	        }
		}

	} while ( (item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu!=MENU_RETORNO_ESC && !salir_todos_menus);
}



#ifdef EMULATE_CPU_STATS

void menu_debug_cpu_stats_clear_disassemble_array(void)
{
	int i;

	for (i=0;i<DISASSEMBLE_ARRAY_LENGTH;i++) disassemble_array[i]=0;
}

void menu_debug_cpu_stats_diss_no_print(z80_byte index,z80_byte opcode,char *dumpassembler)
{

        size_t longitud_opcode;

        disassemble_array[index]=opcode;

        debugger_disassemble_array(dumpassembler,31,&longitud_opcode,0);
}


/*
void menu_debug_cpu_stats_diss(z80_byte index,z80_byte opcode,int linea)
{

	char dumpassembler[32];

	//Empezar con espacio
	dumpassembler[0]=' ';

	menu_debug_cpu_stats_diss_no_print(index,opcode,&dumpassembler[1]);

	menu_escribe_linea_opcion(linea,-1,1,dumpassembler);
}
*/

void menu_debug_cpu_stats_diss_complete_no_print (z80_byte opcode,char *buffer,z80_byte preffix1,z80_byte preffix2)
{

	//Sin prefijo
	if (preffix1==0) {
                        menu_debug_cpu_stats_clear_disassemble_array();
                        menu_debug_cpu_stats_diss_no_print(0,opcode,buffer);
	}

	//Con 1 solo prefijo
	else if (preffix2==0) {
                        menu_debug_cpu_stats_clear_disassemble_array();
                        disassemble_array[0]=preffix1;
                        menu_debug_cpu_stats_diss_no_print(1,opcode,buffer);
	}

	//Con 2 prefijos (DD/FD + CB)
	else {
                        //Opcode
                        menu_debug_cpu_stats_clear_disassemble_array();
                        disassemble_array[0]=preffix1;
                        disassemble_array[1]=preffix2;
                        disassemble_array[2]=0;
                        menu_debug_cpu_stats_diss_no_print(3,opcode,buffer);
	}
}



void menu_cpu_full_stats(unsigned int *stats_table,char *title,z80_byte preffix1,z80_byte preffix2)
{

	int index_op,index_buffer;
	unsigned int counter;

	char stats_buffer[MAX_TEXTO_GENERIC_MESSAGE];

	char dumpassembler[32];

	//margen suficiente para que quepa una linea y un contador int de 32 bits...
	//aunque si pasa el ancho de linea, la rutina de generic_message lo troceara
	char buf_linea[64];

	index_buffer=0;

	for (index_op=0;index_op<256;index_op++) {
		counter=util_stats_get_counter(stats_table,index_op);

		menu_debug_cpu_stats_diss_complete_no_print(index_op,dumpassembler,preffix1,preffix2);

		sprintf (buf_linea,"%02X %-16s: %u \n",index_op,dumpassembler,counter);
		//16 ocupa la instruccion mas larga: LD B,RLC (IX+dd)

		sprintf (&stats_buffer[index_buffer],"%s\n",buf_linea);
		//sprintf (&stats_buffer[index_buffer],"%02X: %11d\n",index_op,counter);
		//index_buffer +=16;
		index_buffer +=strlen(buf_linea);
	}

	stats_buffer[index_buffer]=0;

	menu_generic_message(title,stats_buffer);

}

void menu_cpu_full_stats_codsinpr(MENU_ITEM_PARAMETERS)
{
	menu_cpu_full_stats(stats_codsinpr,"Full statistic no preffix",0,0);
}

void menu_cpu_full_stats_codpred(MENU_ITEM_PARAMETERS)
{
        menu_cpu_full_stats(stats_codpred,"Full statistic pref ED",0xED,0);
}

void menu_cpu_full_stats_codprcb(MENU_ITEM_PARAMETERS)
{
        menu_cpu_full_stats(stats_codprcb,"Full statistic pref CB",0xCB,0);
}

void menu_cpu_full_stats_codprdd(MENU_ITEM_PARAMETERS)
{
        menu_cpu_full_stats(stats_codprdd,"Full statistic pref DD",0xDD,0);
}

void menu_cpu_full_stats_codprfd(MENU_ITEM_PARAMETERS)
{
        menu_cpu_full_stats(stats_codprfd,"Full statistic pref FD",0xFD,0);
}

void menu_cpu_full_stats_codprddcb(MENU_ITEM_PARAMETERS)
{
        menu_cpu_full_stats(stats_codprddcb,"Full statistic pref DDCB",0xDD,0xCB);
}

void menu_cpu_full_stats_codprfdcb(MENU_ITEM_PARAMETERS)
{
        menu_cpu_full_stats(stats_codprfdcb,"Full statistic pref FDCB",0xFD,0xCB);
}


void menu_cpu_full_stats_clear(MENU_ITEM_PARAMETERS)
{
	util_stats_init();

	menu_generic_message("Clear CPU statistics","OK. Statistics cleared");
}


void menu_debug_cpu_stats(MENU_ITEM_PARAMETERS)
{
        menu_item *array_menu_cpu_stats;
        menu_item item_seleccionado;
        int retorno_menu;
        do {
                menu_add_item_menu_inicial_format(&array_menu_cpu_stats,MENU_OPCION_NORMAL,menu_debug_cpu_resumen_stats,NULL,"Compact Statistics");
                menu_add_item_menu_tooltip(array_menu_cpu_stats,"Shows Compact CPU Statistics");
                menu_add_item_menu_ayuda(array_menu_cpu_stats,"Shows the most used opcode for every type: without preffix, with ED preffix, "
					"etc. CPU Statistics are reset when changing machine or resetting CPU.");


                menu_add_item_menu_format(array_menu_cpu_stats,MENU_OPCION_NORMAL,menu_cpu_full_stats_codsinpr,NULL,"Full Statistics No pref");
		menu_add_item_menu_format(array_menu_cpu_stats,MENU_OPCION_NORMAL,menu_cpu_full_stats_codpred,NULL,"Full Statistics Pref ED");
		menu_add_item_menu_format(array_menu_cpu_stats,MENU_OPCION_NORMAL,menu_cpu_full_stats_codprcb,NULL,"Full Statistics Pref CB");
		menu_add_item_menu_format(array_menu_cpu_stats,MENU_OPCION_NORMAL,menu_cpu_full_stats_codprdd,NULL,"Full Statistics Pref DD");
		menu_add_item_menu_format(array_menu_cpu_stats,MENU_OPCION_NORMAL,menu_cpu_full_stats_codprfd,NULL,"Full Statistics Pref FD");
		menu_add_item_menu_format(array_menu_cpu_stats,MENU_OPCION_NORMAL,menu_cpu_full_stats_codprddcb,NULL,"Full Statistics Pref DDCB");
		menu_add_item_menu_format(array_menu_cpu_stats,MENU_OPCION_NORMAL,menu_cpu_full_stats_codprfdcb,NULL,"Full Statistics Pref FDCB");
		menu_add_item_menu_format(array_menu_cpu_stats,MENU_OPCION_NORMAL,menu_cpu_full_stats_clear,NULL,"Clear Statistics");


                menu_add_item_menu(array_menu_cpu_stats,"",MENU_OPCION_SEPARADOR,NULL,NULL);
                //menu_add_item_menu(array_menu_cpu_stats,"ESC Back",MENU_OPCION_NORMAL|MENU_OPCION_ESC,NULL,NULL);
                menu_add_ESC_item(array_menu_cpu_stats);

                retorno_menu=menu_dibuja_menu(&cpu_stats_opcion_seleccionada,&item_seleccionado,array_menu_cpu_stats,"CPU Statistics" );
		

                if ((item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu>=0) {
                        //llamamos por valor de funcion
                        if (item_seleccionado.menu_funcion!=NULL) {
                                //printf ("actuamos por funcion\n");
                                item_seleccionado.menu_funcion(item_seleccionado.valor_opcion);
                                
                        }
                }

        } while ( (item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu!=MENU_RETORNO_ESC && !salir_todos_menus);
}

int cpu_stats_valor_contador_segundo_anterior;

zxvision_window *menu_debug_cpu_resumen_stats_overlay_window;

void menu_debug_cpu_resumen_stats_overlay(void)
{
	if (!zxvision_drawing_in_background) normal_overlay_texto_menu();

	    char textostats[32];
	zxvision_window *ventana;

	ventana=menu_debug_cpu_resumen_stats_overlay_window;


        char dumpassembler[32];

        //Empezar con espacio
        dumpassembler[0]=' ';

				//int valor_contador_segundo_anterior;



		//z80_byte tecla;

		//printf ("%d %d\n",contador_segundo,cpu_stats_valor_contador_segundo_anterior);
     

			//esto hara ejecutar esto 2 veces por segundo
			if ( ((contador_segundo%500) == 0 && cpu_stats_valor_contador_segundo_anterior!=contador_segundo) || menu_multitarea==0) {
											cpu_stats_valor_contador_segundo_anterior=contador_segundo;
				//printf ("Refrescando. contador_segundo=%d\n",contador_segundo);

			int linea=0;
                        int opcode;

			unsigned int sumatotal; 
                        sumatotal=util_stats_sum_all_counters();
                    	sprintf (textostats,"Total opcodes run: %u",sumatotal);
						//menu_escribe_linea_opcion(linea++,-1,1,textostats);
						zxvision_print_string_defaults(ventana,1,linea++,textostats);
                        


						//menu_escribe_linea_opcion(linea++,-1,1,"Most used op. for each preffix");
						zxvision_print_string_defaults(ventana,1,linea++,"Most used op. for each preffix");

                        opcode=util_stats_find_max_counter(stats_codsinpr);
                        sprintf (textostats,"Op nopref:    %02XH: %u",opcode,util_stats_get_counter(stats_codsinpr,opcode) );
						//menu_escribe_linea_opcion(linea++,-1,1,textostats);
						zxvision_print_string_defaults(ventana,1,linea++,textostats);
                        

                        //Opcode
						menu_debug_cpu_stats_diss_complete_no_print(opcode,&dumpassembler[1],0,0);
						//menu_escribe_linea_opcion(linea++,-1,1,dumpassembler);
						zxvision_print_string_defaults(ventana,1,linea++,dumpassembler);
						



                        opcode=util_stats_find_max_counter(stats_codpred);
                        sprintf (textostats,"Op pref ED:   %02XH: %u",opcode,util_stats_get_counter(stats_codpred,opcode) );
						//menu_escribe_linea_opcion(linea++,-1,1,textostats);
						zxvision_print_string_defaults(ventana,1,linea++,textostats);
                        

                        //Opcode
                        menu_debug_cpu_stats_diss_complete_no_print(opcode,&dumpassembler[1],237,0);
						//menu_escribe_linea_opcion(linea++,-1,1,dumpassembler);
						zxvision_print_string_defaults(ventana,1,linea++,dumpassembler);
                        

	
                        opcode=util_stats_find_max_counter(stats_codprcb);
                        sprintf (textostats,"Op pref CB:   %02XH: %u",opcode,util_stats_get_counter(stats_codprcb,opcode) );
						//menu_escribe_linea_opcion(linea++,-1,1,textostats);
						zxvision_print_string_defaults(ventana,1,linea++,textostats);


                        //Opcode
                        menu_debug_cpu_stats_diss_complete_no_print(opcode,&dumpassembler[1],203,0);
						//menu_escribe_linea_opcion(linea++,-1,1,dumpassembler);
						zxvision_print_string_defaults(ventana,1,linea++,dumpassembler);




                        opcode=util_stats_find_max_counter(stats_codprdd);
                        sprintf (textostats,"Op pref DD:   %02XH: %u",opcode,util_stats_get_counter(stats_codprdd,opcode) );
                        //menu_escribe_linea_opcion(linea++,-1,1,textostats);
						zxvision_print_string_defaults(ventana,1,linea++,textostats);

                        //Opcode
                        menu_debug_cpu_stats_diss_complete_no_print(opcode,&dumpassembler[1],221,0);
                        //menu_escribe_linea_opcion(linea++,-1,1,dumpassembler);
						zxvision_print_string_defaults(ventana,1,linea++,dumpassembler);


                        opcode=util_stats_find_max_counter(stats_codprfd);
                        sprintf (textostats,"Op pref FD:   %02XH: %u",opcode,util_stats_get_counter(stats_codprfd,opcode) );
                        //menu_escribe_linea_opcion(linea++,-1,1,textostats);
						zxvision_print_string_defaults(ventana,1,linea++,textostats);

                        //Opcode
                        menu_debug_cpu_stats_diss_complete_no_print(opcode,&dumpassembler[1],253,0);
                        //menu_escribe_linea_opcion(linea++,-1,1,dumpassembler);
						zxvision_print_string_defaults(ventana,1,linea++,dumpassembler);


                        opcode=util_stats_find_max_counter(stats_codprddcb);
                        sprintf (textostats,"Op pref DDCB: %02XH: %u",opcode,util_stats_get_counter(stats_codprddcb,opcode) );
                        //menu_escribe_linea_opcion(linea++,-1,1,textostats);
						zxvision_print_string_defaults(ventana,1,linea++,textostats);

                        //Opcode
                        menu_debug_cpu_stats_diss_complete_no_print(opcode,&dumpassembler[1],221,203);
                        //menu_escribe_linea_opcion(linea++,-1,1,dumpassembler);
						zxvision_print_string_defaults(ventana,1,linea++,dumpassembler);



                        opcode=util_stats_find_max_counter(stats_codprfdcb);
                        sprintf (textostats,"Op pref FDCB: %02XH: %u",opcode,util_stats_get_counter(stats_codprfdcb,opcode) );
                        //menu_escribe_linea_opcion(linea++,-1,1,textostats);
						zxvision_print_string_defaults(ventana,1,linea++,textostats);

                        //Opcode
                        menu_debug_cpu_stats_diss_complete_no_print(opcode,&dumpassembler[1],253,203);
                        //menu_escribe_linea_opcion(linea++,-1,1,dumpassembler);
						zxvision_print_string_defaults(ventana,1,linea++,dumpassembler);


						zxvision_draw_window_contents(ventana);


                }



}

zxvision_window menu_debug_cpu_resumen_stats_ventana;

void menu_debug_cpu_resumen_stats(MENU_ITEM_PARAMETERS)
{

    

	menu_espera_no_tecla();
	menu_reset_counters_tecla_repeticion();		

		zxvision_window *ventana;
		
		ventana=&menu_debug_cpu_resumen_stats_ventana;

	zxvision_new_window(ventana,0,1,32,18,
							31,16,"CPU Compact Statistics");
	zxvision_draw_window(ventana);
		


		menu_debug_cpu_resumen_stats_overlay_window=ventana; //Decimos que el overlay lo hace sobre la ventana que tenemos aqui

						cpu_stats_valor_contador_segundo_anterior=contador_segundo;

        //Cambiamos funcion overlay de texto de menu
        //Se establece a la de funcion de onda + texto
        set_menu_overlay_function(menu_debug_cpu_resumen_stats_overlay);

	z80_byte tecla;

	do {
		tecla=zxvision_common_getkey_refresh();		
		zxvision_handle_cursors_pgupdn(ventana,tecla);
		//printf ("tecla: %d\n",tecla);
	} while (tecla!=2 && tecla!=3);				

	//Gestionar salir con tecla background
 
	menu_espera_no_tecla(); //Si no, se va al menu anterior.
	//En AY Piano por ejemplo esto no pasa aunque el estilo del menu es el mismo...

    //restauramos modo normal de texto de menu
     set_menu_overlay_function(normal_overlay_texto_menu);


    cls_menu_overlay();	


	if (tecla==3) {
		//zxvision_ay_registers_overlay
		ventana->overlay_function=menu_debug_cpu_resumen_stats_overlay;
		printf ("Put window %p in background. next window=%p\n",ventana,ventana->next_window);
		menu_generic_message("Background task","OK. Window put in background");
	}

	else {
		zxvision_destroy_window(ventana);		
 	}




}

#endif

void zxvision_test_sleep_quarter(void)
{
	int previo_contador_segundo=contador_segundo;

	while (1) {
		menu_cpu_core_loop();
		if (previo_contador_segundo!=contador_segundo && (contador_segundo%250)==0) return;

		if (menu_get_pressed_key()!=0) return;
	
	}
}


void menu_zxvision_test(MENU_ITEM_PARAMETERS)
{

        //Desactivamos interlace - si esta. Con interlace la forma de onda se dibuja encima continuamente, sin borrar
        //z80_bit copia_video_interlaced_mode;
        //copia_video_interlaced_mode.v=video_interlaced_mode.v;

        //disable_interlace();


        menu_espera_no_tecla();


	//Prueba filesel
       /* char *filtros[2];

        filtros[0]="";
        filtros[1]=0;

        char nombredestino[PATH_MAX];
	nombredestino[0]=0;


	int retorno=menu_filesel("Select Target File",filtros,nombredestino);
	printf ("retorno: %d nombredestino: [%s]\n",retorno,nombredestino);
	return;*/



		//zxvision_generic_message_tooltip("pruebas", 30, 0, 0, generic_message_tooltip_return *retorno, const char * texto_format , ...)
		zxvision_generic_message_tooltip("Pruebas" , 0 , 0, 0, 0, NULL, 0, "Hola que tal como estas esto es una prueba de escribir texto. "
					"No se que mas poner pero me voy a empezar a repetir, " 
					"Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore "
					"et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip"
					" ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore "
					" eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia "
					"deserunt mollit anim id est laborum. Adios");
        //z80_byte acumulado;



        //Cambiamos funcion overlay de texto de menu
        //Se establece a la de funcion de audio waveform
	//set_menu_overlay_function(menu_audio_draw_sound_wave);

	zxvision_window ventana;

#define SOUND_ZXVISION_WAVE_X 1
#define SOUND_ZXVISION_WAVE_Y 3
#define SOUND_ZXVISION_WAVE_ANCHO 27
#define SOUND_ZXVISION_WAVE_ALTO 14

	int ancho_visible=SOUND_ZXVISION_WAVE_ANCHO;
	int alto_visible=SOUND_ZXVISION_WAVE_ALTO+4;

	int ancho_total=20;
	int alto_total=alto_visible+2;

	//menu_item *array_menu_audio_new_waveform;
      //  menu_item item_seleccionado;
        //int retorno_menu;
        


	  //Hay que redibujar la ventana desde este bucle
	//menu_dibuja_ventana(SOUND_WAVE_X,SOUND_WAVE_Y-2,SOUND_WAVE_ANCHO,SOUND_WAVE_ALTO+4,"Waveform");
	zxvision_new_window(&ventana,SOUND_ZXVISION_WAVE_X,SOUND_ZXVISION_WAVE_Y-2,ancho_visible,alto_visible,
							ancho_total,alto_total,"ZXVision Test");
	zxvision_draw_window(&ventana);

	printf ("Created window\n");

	menu_espera_tecla();
	menu_espera_no_tecla();

	zxvision_draw_window_contents(&ventana);

	printf ("Drawn window contents\n");

	menu_espera_tecla(); 
	menu_espera_no_tecla();


	overlay_screen caracter;

/*	struct s_overlay_screen {
	z80_byte tinta,papel,parpadeo;
	z80_byte caracter;
};*/

	caracter.tinta=ESTILO_GUI_TINTA_NORMAL;
	caracter.papel=ESTILO_GUI_PAPEL_NORMAL;
	caracter.parpadeo=0;

	//Relleno pantalla
	z80_byte caracter_print=32;



	int x,y;

	for (y=0;y<alto_total;y++) {
		for (x=0;x<ancho_total;x++) {
			caracter.caracter=caracter_print;
			zxvision_print_char(&ventana,x,y,&caracter);	

			caracter_print++;
			if (caracter_print>126) caracter_print=32;		
		}
	}

	caracter.caracter='A';

	zxvision_print_char(&ventana,0,0,&caracter);

	caracter.caracter='B';

	zxvision_print_char(&ventana,0,1,&caracter);

	zxvision_print_string(&ventana,0,0,ESTILO_GUI_TINTA_NORMAL,ESTILO_GUI_PAPEL_NORMAL,0," Texto leyenda 1 ");
	zxvision_print_string(&ventana,0,1,ESTILO_GUI_TINTA_NORMAL,ESTILO_GUI_PAPEL_NORMAL,0," Texto leyenda 2 ");
	zxvision_print_string(&ventana,0,2,ESTILO_GUI_TINTA_NORMAL,ESTILO_GUI_PAPEL_NORMAL,0," Texto leyenda 3 ");

	zxvision_print_string(&ventana,2,3,ESTILO_GUI_PAPEL_NORMAL,ESTILO_GUI_TINTA_NORMAL,1," This is a test ");

	zxvision_print_string(&ventana,2,4,ESTILO_GUI_PAPEL_NORMAL,ESTILO_GUI_TINTA_NORMAL,1," Press a key ");
	zxvision_print_string(&ventana,2,5,ESTILO_GUI_PAPEL_NORMAL,ESTILO_GUI_TINTA_NORMAL,1," to next step ");

	zxvision_print_string(&ventana,2,6,ESTILO_GUI_TINTA_NORMAL,ESTILO_GUI_PAPEL_NORMAL,0," --^^flash^^--");
	zxvision_print_string(&ventana,2,7,ESTILO_GUI_TINTA_NORMAL,ESTILO_GUI_PAPEL_NORMAL,0," --~~inverse--");
	zxvision_print_string(&ventana,2,8,ESTILO_GUI_TINTA_NORMAL,ESTILO_GUI_PAPEL_NORMAL,0," --$$2ink--");


                    /*    menu_add_item_menu_inicial_format(&array_menu_audio_new_waveform,MENU_OPCION_NORMAL,menu_audio_new_waveform_shape,NULL,"Change wave ~~Shape");
                        menu_add_item_menu_shortcut(array_menu_audio_new_waveform,'s');

                        //Evito tooltips en los menus tabulados que tienen overlay porque al salir el tooltip detiene el overlay
                        //menu_add_item_menu_tooltip(array_menu_audio_new_waveform,"Change wave Shape");
                        menu_add_item_menu_ayuda(array_menu_audio_new_waveform,"Change wave Shape: simple line or vertical fill");
						//0123456789
						// Change wave Shape
						
			menu_add_item_menu_tabulado(array_menu_audio_new_waveform,1,0);





		//Nombre de ventana solo aparece en el caso de stdout
                retorno_menu=menu_dibuja_menu(&audio_new_waveform_opcion_seleccionada,&item_seleccionado,array_menu_audio_new_waveform,"Waveform" );*/


	ventana.upper_margin=2;
	ventana.lower_margin=1;


	menu_espera_tecla();
	menu_espera_no_tecla();

	zxvision_draw_window_contents(&ventana);

	menu_espera_tecla(); 
	menu_espera_no_tecla();

	//Jugar con offset
/*	int i;

	for (i=0;i<7;i++) {
		zxvision_set_offset_x(&ventana,i);

		zxvision_draw_window_contents(&ventana);

		printf ("Offset x %d\n",i);

		menu_espera_tecla();
		menu_espera_no_tecla();		
	}


	for (i=0;i<7;i++) {
		zxvision_set_offset_y(&ventana,i);

		zxvision_draw_window_contents(&ventana);

		printf ("Offset y %d\n",i);

		menu_espera_tecla();
		menu_espera_no_tecla();		
	}



	for (i=0;i<10;i++) {
		zxvision_set_x_position(&ventana,i);

		printf ("Move x %d\n",i);

		menu_espera_tecla();
		menu_espera_no_tecla();		
	}

	for (i=0;i<10;i++) {
		zxvision_set_y_position(&ventana,i);

		printf ("Move y %d\n",i);

		menu_espera_tecla();
		menu_espera_no_tecla();		
	}

	zxvision_set_x_position(&ventana,0);
	zxvision_set_y_position(&ventana,0);

	for (i=25;i<35;i++) {
		zxvision_set_visible_width(&ventana,i);

		printf ("width %d\n",i);

		menu_espera_tecla();
		menu_espera_no_tecla();		
	}	

	for (i=18;i<28;i++) {
		zxvision_set_visible_height(&ventana,i);

		printf ("height %d\n",i);

		menu_espera_tecla();
		menu_espera_no_tecla();		
	}	


	for (i=5;i>=0;i--) {
		zxvision_set_visible_width(&ventana,i);

		printf ("width %d\n",i);

		menu_espera_tecla();
		menu_espera_no_tecla();		
	}	

	zxvision_set_visible_width(&ventana,20);

	for (i=5;i>=0;i--) {
		zxvision_set_visible_height(&ventana,i);

		printf ("height %d\n",i);

		menu_espera_tecla();
		menu_espera_no_tecla();		
	}	


*/
	zxvision_print_string(&ventana,2,3,ESTILO_GUI_PAPEL_NORMAL,ESTILO_GUI_TINTA_NORMAL,1," Use cursors ");
	zxvision_print_string(&ventana,2,4,ESTILO_GUI_PAPEL_NORMAL,ESTILO_GUI_TINTA_NORMAL,1," to move offset ");	
	zxvision_print_string(&ventana,2,5,ESTILO_GUI_PAPEL_NORMAL,ESTILO_GUI_TINTA_NORMAL,1," QAOP size");	
	zxvision_print_string(&ventana,2,6,ESTILO_GUI_PAPEL_NORMAL,ESTILO_GUI_TINTA_NORMAL,1," ESC exit ");	


	//Rebotar
	int contador=0;

	int xpos=0;
	int ypos=0;

	int incx=+1;
	int incy=+1;

	int offsetx=0;
	int offsety=0;	

	int ancho=22;
	int alto=10;

	zxvision_set_visible_height(&ventana,alto);
	zxvision_set_visible_width(&ventana,ancho);	

	z80_byte tecla=0;

	//Salir con ESC
	while (tecla!=2) {

		zxvision_set_offset_x(&ventana,offsetx);
		zxvision_set_offset_y(&ventana,offsety);

		zxvision_set_x_position(&ventana,xpos);
		zxvision_set_y_position(&ventana,ypos);		

		zxvision_test_sleep_quarter();

		tecla=menu_get_pressed_key();
		//Cambio offset con cursores
		if (tecla==8) {
			offsetx--;
			printf ("Decrement offset x to %d\n",offsetx);
		}

		if (tecla==9) {
			offsetx++;
			printf ("Increment offset x to %d\n",offsetx);
		}

		if (tecla==10) {
			offsety++;
			printf ("Increment offset y to %d\n",offsety);
		}

		if (tecla==11) {
			offsety--;
			printf ("Decrement offset y to %d\n",offsety);
		}

		//Cambio tamanyo
		if (tecla=='a' && ypos+alto<24) {
			alto++;
			printf ("Increment height to %d\n",alto);
			zxvision_set_visible_height(&ventana,alto);
		}

		if (tecla=='q' && alto>1) {
			alto--;
			printf ("Decrement height to %d\n",alto);
			zxvision_set_visible_height(&ventana,alto);
		}

		if (tecla=='p' && xpos+ancho<32) {
			ancho++;
			printf ("Increment width to %d\n",ancho);
			zxvision_set_visible_width(&ventana,ancho);
		}

		if (tecla=='o' && ancho>7) {
			ancho--;
			printf ("Decrement width to %d\n",ancho);
			zxvision_set_visible_width(&ventana,ancho);
		}

		xpos +=incx;
		if (xpos+ancho>=32 || xpos<=0) {
			incx=-incx;
		}

		ypos +=incy;
		if (ypos+alto>=24 || ypos<=0) {
			incy=-incy;
		}		
		
		contador++;

		if (tecla!=0) menu_espera_no_tecla();

	}


	zxvision_print_string(&ventana,1,5,ESTILO_GUI_PAPEL_NORMAL,ESTILO_GUI_TINTA_NORMAL,1," Use mouse    ");
	zxvision_print_string(&ventana,1,6,ESTILO_GUI_PAPEL_NORMAL,ESTILO_GUI_TINTA_NORMAL,1," to move and   ");	
	zxvision_print_string(&ventana,1,7,ESTILO_GUI_PAPEL_NORMAL,ESTILO_GUI_TINTA_NORMAL,1," resize window  ");	
	zxvision_print_string(&ventana,1,8,ESTILO_GUI_PAPEL_NORMAL,ESTILO_GUI_TINTA_NORMAL,1," Right button exits ");	

	zxvision_draw_window_contents(&ventana);

	tecla=0;

	while (!mouse_right) {
		//menu_espera_tecla();
		tecla=menu_get_pressed_key();
		//Comprobar eventos raton
		menu_cpu_core_loop();
		//zxvision_handle_mouse_events(&ventana);
	} 

    cls_menu_overlay();
	zxvision_destroy_window(&ventana);
            




}


void menu_about_core_statistics(MENU_ITEM_PARAMETERS)
{

    menu_espera_no_tecla();
	menu_reset_counters_tecla_repeticion();


	zxvision_window ventana;

	zxvision_new_window(&ventana,0,7,32,9,
							31,7,"Core Statistics");

	zxvision_draw_window(&ventana);


        char texto_buffer[33];


        //Empezar con espacio
    texto_buffer[0]=' ';

        int valor_contador_segundo_anterior;

        valor_contador_segundo_anterior=contador_segundo;


		z80_byte tecla;

        do {


                //esto hara ejecutar esto 2 veces por segundo
                if ( ((contador_segundo%500) == 0 && valor_contador_segundo_anterior!=contador_segundo) || menu_multitarea==0) {
                     valor_contador_segundo_anterior=contador_segundo;
                     //printf ("Refrescando. contador_segundo=%d\n",contador_segundo);

                                int linea=0;
                                //int opcode;
                                //int sumatotal;

/*

Nota: calcular el tiempo entre ejecuciones de cada opcode no penaliza mucho el uso de cpu real.
Ejemplo:
--vo null --machine 48k 

Sin calcular ese tiempo: 9% cpu
Calculando ese tiempo: 12% cpu

*/


//Ultimo intervalo de tiempo
//long core_cpu_timer_frame_difftime;
//Media de todos los intervalos
//long core_cpu_timer_frame_media=0;

				long valor_mostrar;
				valor_mostrar=core_cpu_timer_frame_difftime;
				//controlar maximos
				if (valor_mostrar>999999) valor_mostrar=999999;
			     //01234567890123456789012345678901
			     // Last core frame: 999999 us
				sprintf (texto_buffer,"Last core frame:     %6ld us",valor_mostrar);
				//menu_escribe_linea_opcion(linea++,-1,1,texto_buffer);	
				zxvision_print_string_defaults(&ventana,1,linea++,texto_buffer);

                                valor_mostrar=core_cpu_timer_frame_media;
                                //controlar maximos
				if (valor_mostrar>999999) valor_mostrar=999999;
                                //01234567890123456789012345678901
                                 // Last core frame: 999999 us
                                sprintf (texto_buffer," Average:   %6ld us",valor_mostrar);
                                //menu_escribe_linea_opcion(linea++,-1,1,texto_buffer);
								zxvision_print_string_defaults(&ventana,1,linea++,texto_buffer);


                                valor_mostrar=core_cpu_timer_refresca_pantalla_difftime;
                                //controlar maximos
                                if (valor_mostrar>999999) valor_mostrar=999999;
                             //01234567890123456789012345678901
                             // Last render display: 999999 us
                                sprintf (texto_buffer,"Last full render:    %6ld us",valor_mostrar);
                                //menu_escribe_linea_opcion(linea++,-1,1,texto_buffer);
								zxvision_print_string_defaults(&ventana,1,linea++,texto_buffer);

                                valor_mostrar=core_cpu_timer_refresca_pantalla_media;
                                //controlar maximos
                                if (valor_mostrar>999999) valor_mostrar=999999;
                                //01234567890123456789012345678901
                                 // Last core refresca_pantalla: 999999 us
                                sprintf (texto_buffer," Average:   %6ld us",valor_mostrar);
                                //menu_escribe_linea_opcion(linea++,-1,1,texto_buffer);
								zxvision_print_string_defaults(&ventana,1,linea++,texto_buffer);


                                valor_mostrar=core_cpu_timer_each_frame_difftime;
                                //controlar maximos
                                if (valor_mostrar>999999) valor_mostrar=999999;
                             //01234567890123456789012345678901
                             // Time between frames: 999999 us
                                sprintf (texto_buffer,"Time between frames: %6ld us",valor_mostrar);
                                //menu_escribe_linea_opcion(linea++,-1,1,texto_buffer);
								zxvision_print_string_defaults(&ventana,1,linea++,texto_buffer);

                                valor_mostrar=core_cpu_timer_each_frame_media;
                                //controlar maximos
                                if (valor_mostrar>999999) valor_mostrar=999999;
                                //01234567890123456789012345678901
                                 // Last core each_frame: 999999 us
                                sprintf (texto_buffer," Average:   %6ld us",valor_mostrar);
                                //menu_escribe_linea_opcion(linea++,-1,1,texto_buffer);
								zxvision_print_string_defaults(&ventana,1,linea++,texto_buffer);

								 //menu_escribe_linea_opcion(linea++,-1,1," (ideal):  20000 us");
								 zxvision_print_string_defaults(&ventana,1,linea++," (expected): 20000 us");


								zxvision_draw_window_contents(&ventana);


                }

				//Nota: No usamos zxvision_common_getkey_refresh porque necesitamos que el bucle se ejecute continuamente para poder 
				//refrescar contenido de ventana, dado que aqui no llamamos a menu_espera_tecla
				//(a no ser que este multitarea off)
				tecla=zxvision_common_getkey_refresh_noesperatecla();				


				zxvision_handle_cursors_pgupdn(&ventana,tecla);


		} while (tecla!=2);

        cls_menu_overlay();
		zxvision_destroy_window(&ventana);


}




int ayregisters_previo_valor_volume_A[MAX_AY_CHIPS];
int ayregisters_previo_valor_volume_B[MAX_AY_CHIPS];
int ayregisters_previo_valor_volume_C[MAX_AY_CHIPS];

	int menu_ayregisters_valor_contador_segundo_anterior;

zxvision_window *menu_ay_registers_overlay_window;

void menu_ay_registers_overlay(void)
{

	//NOTA: //Hemos de suponer que current window es esta de ay registers

    if (!zxvision_drawing_in_background) normal_overlay_texto_menu();

	char volumen[32],textotono[32];
	char textovolumen[35]; //32+3 de posible color rojo del maximo


	int total_chips=ay_retorna_numero_chips();

	//Como maximo mostrarmos 3 canales ay
	if (total_chips>3) total_chips=3;

	int chip;

	int linea=0;

	menu_speech_tecla_pulsada=1; //Si no, envia continuamente todo ese texto a speech

		int vol_A[MAX_AY_CHIPS],vol_B[MAX_AY_CHIPS],vol_C[MAX_AY_CHIPS];

	for (chip=0;chip<total_chips;chip++) {


        	vol_A[chip]=ay_3_8912_registros[chip][8] & 15;
        	vol_B[chip]=ay_3_8912_registros[chip][9] & 15;
        	vol_C[chip]=ay_3_8912_registros[chip][10] & 15;

			//Controlar limites, dado que las variables entran sin inicializar
			if (ayregisters_previo_valor_volume_A[chip]>16) ayregisters_previo_valor_volume_A[chip]=16;
			if (ayregisters_previo_valor_volume_B[chip]>16) ayregisters_previo_valor_volume_B[chip]=16;
			if (ayregisters_previo_valor_volume_C[chip]>16) ayregisters_previo_valor_volume_C[chip]=16;
			

			ayregisters_previo_valor_volume_A[chip]=menu_decae_ajusta_valor_volumen(ayregisters_previo_valor_volume_A[chip],vol_A[chip]);
			ayregisters_previo_valor_volume_B[chip]=menu_decae_ajusta_valor_volumen(ayregisters_previo_valor_volume_B[chip],vol_B[chip]);
			ayregisters_previo_valor_volume_C[chip]=menu_decae_ajusta_valor_volumen(ayregisters_previo_valor_volume_C[chip],vol_C[chip]);


        		//if (ayregisters_previo_valor_volume_A[chip]<vol_A[chip]) ayregisters_previo_valor_volume_A[chip]=vol_A[chip];
        		//if (ayregisters_previo_valor_volume_B[chip]<vol_B[chip]) ayregisters_previo_valor_volume_B[chip]=vol_B[chip];
        		//if (ayregisters_previo_valor_volume_C[chip]<vol_C[chip]) ayregisters_previo_valor_volume_C[chip]=vol_C[chip];


			menu_string_volumen(volumen,ay_3_8912_registros[chip][8],ayregisters_previo_valor_volume_A[chip]);
			sprintf (textovolumen,"Volume A: %s",volumen);
			//menu_escribe_linea_opcion(linea++,-1,1,textovolumen);
			zxvision_print_string_defaults(menu_ay_registers_overlay_window,1,linea++,textovolumen);

			menu_string_volumen(volumen,ay_3_8912_registros[chip][9],ayregisters_previo_valor_volume_B[chip]);
			sprintf (textovolumen,"Volume B: %s",volumen);
			//menu_escribe_linea_opcion(linea++,-1,1,textovolumen);
			zxvision_print_string_defaults(menu_ay_registers_overlay_window,1,linea++,textovolumen);

			menu_string_volumen(volumen,ay_3_8912_registros[chip][10],ayregisters_previo_valor_volume_C[chip]);
			sprintf (textovolumen,"Volume C: %s",volumen);
			//menu_escribe_linea_opcion(linea++,-1,1,textovolumen);
			zxvision_print_string_defaults(menu_ay_registers_overlay_window,1,linea++,textovolumen);



			int freq_a=ay_retorna_frecuencia(0,chip);
			int freq_b=ay_retorna_frecuencia(1,chip);
			int freq_c=ay_retorna_frecuencia(2,chip);
			sprintf (textotono,"Channel A:  %3s %7d Hz",get_note_name(freq_a),freq_a);
			//menu_escribe_linea_opcion(linea++,-1,1,textotono);
			zxvision_print_string_defaults(menu_ay_registers_overlay_window,1,linea++,textotono);

			sprintf (textotono,"Channel B:  %3s %7d Hz",get_note_name(freq_b),freq_b);
			//menu_escribe_linea_opcion(linea++,-1,1,textotono);
			zxvision_print_string_defaults(menu_ay_registers_overlay_window,1,linea++,textotono);

			sprintf (textotono,"Channel C:  %3s %7d Hz",get_note_name(freq_c),freq_c);
			//menu_escribe_linea_opcion(linea++,-1,1,textotono);
			zxvision_print_string_defaults(menu_ay_registers_overlay_window,1,linea++,textotono);


			//Si hay 3 canales, los 3 siguientes items no se ven
			if (total_chips<3) {

			                        //Frecuencia ruido
                        int freq_temp=ay_3_8912_registros[chip][6] & 31;
                        //printf ("Valor registros ruido : %d Hz\n",freq_temp);
                        freq_temp=freq_temp*16;

                        //controlamos divisiones por cero
                        if (!freq_temp) freq_temp++;

                        int freq_ruido=FRECUENCIA_NOISE/freq_temp;

                        sprintf (textotono,"Frequency Noise: %6d Hz",freq_ruido);
                        //menu_escribe_linea_opcion(linea++,-1,1,textotono);
						zxvision_print_string_defaults(menu_ay_registers_overlay_window,1,linea++,textotono);


			//Envelope

                        freq_temp=ay_3_8912_registros[chip][11]+256*(ay_3_8912_registros[chip][12] & 0xFF);


                        //controlamos divisiones por cero
                        if (!freq_temp) freq_temp++;
                        int freq_envelope=FRECUENCIA_ENVELOPE/freq_temp;

                        //sprintf (textotono,"Freq Envelope(*10): %5d Hz",freq_envelope);

			int freq_env_10=freq_envelope/10;
			int freq_env_decimal=freq_envelope-(freq_env_10*10);

			sprintf (textotono,"Freq Envelope:   %4d.%1d Hz",freq_env_10,freq_env_decimal);
      		//menu_escribe_linea_opcion(linea++,-1,1,textotono);
			zxvision_print_string_defaults(menu_ay_registers_overlay_window,1,linea++,textotono);



			char envelope_name[32];
			z80_byte env_type=ay_3_8912_registros[chip][13] & 0x0F;
			return_envelope_name(env_type,envelope_name);
			sprintf (textotono,"Env.: %2d (%s)",env_type,envelope_name);
            //menu_escribe_linea_opcion(linea++,-1,1,textotono);
			zxvision_print_string_defaults(menu_ay_registers_overlay_window,1,linea++,textotono);


			}



			sprintf (textotono,"Tone Channels:  %c %c %c",
				( (ay_3_8912_registros[chip][7]&1)==0 ? 'A' : ' '),
				( (ay_3_8912_registros[chip][7]&2)==0 ? 'B' : ' '),
				( (ay_3_8912_registros[chip][7]&4)==0 ? 'C' : ' '));
			//menu_escribe_linea_opcion(linea++,-1,1,textotono);
			zxvision_print_string_defaults(menu_ay_registers_overlay_window,1,linea++,textotono);

			//Si hay 3 canales, los 3 siguientes items no se ven
			if (total_chips<3) {

                        sprintf (textotono,"Noise Channels: %c %c %c",
                                ( (ay_3_8912_registros[chip][7]&8)==0  ? 'A' : ' '),
                                ( (ay_3_8912_registros[chip][7]&16)==0 ? 'B' : ' '),
                                ( (ay_3_8912_registros[chip][7]&32)==0 ? 'C' : ' '));
                        //menu_escribe_linea_opcion(linea++,-1,1,textotono);
						zxvision_print_string_defaults(menu_ay_registers_overlay_window,1,linea++,textotono);
			}

	}




	//Hacer decaer volumenes
                        //Decrementar volumenes que caen, pero hacerlo no siempre, sino 2 veces por segundo
            //esto hara ejecutar esto 2 veces por segundo
            if ( ((contador_segundo%500) == 0 && menu_ayregisters_valor_contador_segundo_anterior!=contador_segundo) || menu_multitarea==0) {

                                 menu_ayregisters_valor_contador_segundo_anterior=contador_segundo;
                                //printf ("Refrescando. contador_segundo=%d. chip: %d\n",contador_segundo,chip);

				for (chip=0;chip<total_chips;chip++) {

					ayregisters_previo_valor_volume_A[chip]=menu_decae_dec_valor_volumen(ayregisters_previo_valor_volume_A[chip],vol_A[chip]);
					ayregisters_previo_valor_volume_B[chip]=menu_decae_dec_valor_volumen(ayregisters_previo_valor_volume_B[chip],vol_B[chip]);
					ayregisters_previo_valor_volume_C[chip]=menu_decae_dec_valor_volumen(ayregisters_previo_valor_volume_C[chip],vol_C[chip]);


				}


        }


	zxvision_draw_window_contents(menu_ay_registers_overlay_window); 


}


//Ventana como variable global
zxvision_window zxvision_ay_registers_overlay;

void menu_ay_registers(MENU_ITEM_PARAMETERS)
{
    menu_espera_no_tecla();
	menu_reset_counters_tecla_repeticion();

		if (!menu_multitarea) {
			menu_warn_message("This menu item needs multitask enabled");
			return;
		}

		int total_chips=ay_retorna_numero_chips();
		if (total_chips>3) total_chips=3;

		int yventana;
		int alto_ventana;

        if (total_chips==1) {
			yventana=5;
			alto_ventana=14;
		}
		else {
			yventana=0;
			alto_ventana=24;
		}

		zxvision_window *ventana;
		ventana=&zxvision_ay_registers_overlay;

		zxvision_new_window(ventana,1,yventana,30,alto_ventana,
							30-1,alto_ventana-2,"AY Registers");

		zxvision_draw_window(ventana);		




        //Cambiamos funcion overlay de texto de menu
        //Se establece a la de funcion de onda + texto
        set_menu_overlay_function(menu_ay_registers_overlay);

		menu_ay_registers_overlay_window=ventana; //Decimos que el overlay lo hace sobre la ventana que tenemos aqui


	z80_byte tecla;

	do {
		tecla=zxvision_common_getkey_refresh();		
		zxvision_handle_cursors_pgupdn(ventana,tecla);
		//printf ("tecla: %d\n",tecla);
	} while (tecla!=2 && tecla!=3);				

	//Gestionar salir con tecla background
 
	menu_espera_no_tecla(); //Si no, se va al menu anterior.
	//En AY Piano por ejemplo esto no pasa aunque el estilo del menu es el mismo...

    //restauramos modo normal de texto de menu
     set_menu_overlay_function(normal_overlay_texto_menu);


    cls_menu_overlay();	


	if (tecla==3) {
		//zxvision_ay_registers_overlay
		ventana->overlay_function=menu_ay_registers_overlay;
		printf ("Put window %p in background. next window=%p\n",ventana,ventana->next_window);
		menu_generic_message("Background task","OK. Window put in background");
	}

	else {
		zxvision_destroy_window(ventana);		
 	}
}



void menu_draw_background_windows_overlay(void)
{

	//menu_ay_registers_overlay();
	//return;
	normal_overlay_texto_menu();

	zxvision_window *ventana;
	ventana=zxvision_current_window;
	zxvision_draw_below_windows_with_overlay(ventana);
	printf ("overlay funcion desde menu_draw_background_windows_overlay\n");
}

void menu_draw_background_windows(MENU_ITEM_PARAMETERS)
{
	menu_espera_no_tecla();
        menu_reset_counters_tecla_repeticion();

                if (!menu_multitarea) {
                        menu_warn_message("This menu item needs multitask enabled");
                        return;
                }

	if (zxvision_current_window==NULL) {
		printf ("No windows in background\n");
		return;
	}

                zxvision_window *ventana;
                ventana=zxvision_current_window;

	//Metemos funcion de overlay que se encarga de repintar ventanas de debajo con overlay
	set_menu_overlay_function(menu_draw_background_windows_overlay);


        z80_byte tecla;

        do {
                tecla=zxvision_common_getkey_refresh();

                printf ("tecla: %d\n",tecla);
        } while (tecla!=2);


        menu_espera_no_tecla(); //Si no, se va al menu anterior.

    //restauramos modo normal de texto de menu
     set_menu_overlay_function(normal_overlay_texto_menu);


    cls_menu_overlay();


}



void menu_debug_tsconf_tbblue_videoregisters(MENU_ITEM_PARAMETERS)
{

	menu_espera_no_tecla();
	menu_reset_counters_tecla_repeticion();

	int xventana=0;
	int ancho_ventana=32;

	int yventana;
	int alto_ventana;
    

	if (MACHINE_IS_TBBLUE) {
		yventana=0;
		alto_ventana=24;
	}

	else {
		yventana=7;
		alto_ventana=8;
	}

	zxvision_window ventana;

		zxvision_new_window(&ventana,xventana,yventana,ancho_ventana,alto_ventana,
							ancho_ventana-1,alto_ventana-2,"Video Info");							

	zxvision_draw_window(&ventana);




	char texto_buffer[64];

	char texto_buffer2[64];

	//Empezar con espacio
    texto_buffer[0]=' ';

	int valor_contador_segundo_anterior;

	valor_contador_segundo_anterior=contador_segundo;

	z80_byte tecla;


    	do {


        	//esto hara ejecutar esto 2 veces por segundo
			if ( ((contador_segundo%500) == 0 && valor_contador_segundo_anterior!=contador_segundo) || menu_multitarea==0) {
											valor_contador_segundo_anterior=contador_segundo;
				//printf ("Refrescando. contador_segundo=%d\n",contador_segundo);

				int linea=0;
				//int opcode;
				//int sumatotal;


				if (MACHINE_IS_TSCONF) {

				int vpage_addr=tsconf_get_vram_page()*16384;

				tsconf_get_current_video_mode(texto_buffer2);
				sprintf (texto_buffer,"Video mode: %s",texto_buffer2);
				//menu_escribe_linea_opcion(linea++,-1,1,texto_buffer);
				zxvision_print_string_defaults(&ventana,1,linea++,texto_buffer);
				
                sprintf (texto_buffer,"Video addr: %06XH",vpage_addr);
                //menu_escribe_linea_opcion(linea++,-1,1,texto_buffer);
				zxvision_print_string_defaults(&ventana,1,linea++,texto_buffer);

				sprintf (texto_buffer,"Tile Map Page: %06XH",tsconf_return_tilemappage() );
				//menu_escribe_linea_opcion(linea++,-1,1,texto_buffer);
				zxvision_print_string_defaults(&ventana,1,linea++,texto_buffer);

				sprintf (texto_buffer,"Tile 0 Graphics addr: %06XH",tsconf_return_tilegraphicspage(0) );
				//menu_escribe_linea_opcion(linea++,-1,1,texto_buffer);
				zxvision_print_string_defaults(&ventana,1,linea++,texto_buffer);

				sprintf (texto_buffer,"Tile 1 Graphics addr: %06XH",tsconf_return_tilegraphicspage(1) );
				//menu_escribe_linea_opcion(linea++,-1,1,texto_buffer);
				zxvision_print_string_defaults(&ventana,1,linea++,texto_buffer);


				sprintf (texto_buffer,"Sprite Graphics addr: %06XH",tsconf_return_spritesgraphicspage() );
				//menu_escribe_linea_opcion(linea++,-1,1,texto_buffer);
				zxvision_print_string_defaults(&ventana,1,linea++,texto_buffer);

				}

				if (MACHINE_IS_TBBLUE) {

					//menu_escribe_linea_opcion(linea++,-1,1,"ULA Video mode:");		
					zxvision_print_string_defaults(&ventana,1,linea++,"ULA Video mode:");

					//menu_escribe_linea_opcion(linea++,-1,1,get_spectrum_ula_string_video_mode() );
					zxvision_print_string_defaults(&ventana,1,linea++,get_spectrum_ula_string_video_mode() );

					linea++;

					//menu_escribe_linea_opcion(linea++,-1,1,"Palette format:");
					zxvision_print_string_defaults(&ventana,1,linea++,"Palette format:");

					tbblue_get_string_palette_format(texto_buffer);
					//menu_escribe_linea_opcion(linea++,-1,1,texto_buffer);
					zxvision_print_string_defaults(&ventana,1,linea++,texto_buffer);

					linea++;

					/*
					(R/W) 0x12 (18) => Layer 2 RAM page
 bits 7-6 = Reserved, must be 0
 bits 5-0 = SRAM page (point to page 8 after a Reset)

(R/W) 0x13 (19) => Layer 2 RAM shadow page
 bits 7-6 = Reserved, must be 0
 bits 5-0 = SRAM page (point to page 11 after a Reset)
					*/

				//tbblue_get_offset_start_layer2_reg
					sprintf (texto_buffer,"Layer 2 addr:        %06XH",tbblue_get_offset_start_layer2_reg(tbblue_registers[18]) );
					zxvision_print_string_defaults(&ventana,1,linea++,texto_buffer);

					sprintf (texto_buffer,"Layer 2 shadow addr: %06XH",tbblue_get_offset_start_layer2_reg(tbblue_registers[19]) );					
					zxvision_print_string_defaults(&ventana,1,linea++,texto_buffer);

					sprintf (texto_buffer,"Tilemap base addr:     %02X00H",0x40+tbblue_get_offset_start_tilemap() );					
					zxvision_print_string_defaults(&ventana,1,linea++,texto_buffer);

					sprintf (texto_buffer,"Tile definitions addr: %02X00H",0x40+tbblue_get_offset_start_tiledef() );					
					zxvision_print_string_defaults(&ventana,1,linea++,texto_buffer);					

					sprintf (texto_buffer,"Tile width: %d columns",tbblue_get_tilemap_width() );					
					zxvision_print_string_defaults(&ventana,1,linea++,texto_buffer);							

					/*
					z80_byte clip_windows[TBBLUE_CLIP_WINDOW_LAYER2][4];
z80_byte clip_windows[TBBLUE_CLIP_WINDOW_SPRITES][4];
z80_byte clip_windows[TBBLUE_CLIP_WINDOW_ULA][4];
z80_byte clip_windows[TBBLUE_CLIP_WINDOW_TILEMAP][4];
					*/

					linea++;
					sprintf (texto_buffer,"Clip Windows:");
					zxvision_print_string_defaults(&ventana,1,linea++,texto_buffer);

					sprintf (texto_buffer,"Layer2:  X=%3d-%3d Y=%3d-%3d",
					clip_windows[TBBLUE_CLIP_WINDOW_LAYER2][0],clip_windows[TBBLUE_CLIP_WINDOW_LAYER2][1],clip_windows[TBBLUE_CLIP_WINDOW_LAYER2][2],clip_windows[TBBLUE_CLIP_WINDOW_LAYER2][3]);
					zxvision_print_string_defaults(&ventana,1,linea++,texto_buffer);



//                    //overwrite currently selected clip-window index value by "selection" graphics
//                    const static int clip_index_string_pos_x[4] = { 11, 15, 21, 25};
//                    int clip_select_x = clip_index_string_pos_x[tbblue_get_clip_window_layer2_index()];
//                    texto_buffer[clip_select_x+3] = 0;      // display only three digits in new colour
//					zxvision_print_string(&ventana,1+clip_select_x,linea++,
//                                          ESTILO_GUI_TINTA_NORMAL,ESTILO_GUI_PAPEL_SELECCIONADO,0,texto_buffer+clip_select_x);


					sprintf (texto_buffer,"Sprites: X=%3d-%3d Y=%3d-%3d",
					clip_windows[TBBLUE_CLIP_WINDOW_SPRITES][0],clip_windows[TBBLUE_CLIP_WINDOW_SPRITES][1],clip_windows[TBBLUE_CLIP_WINDOW_SPRITES][2],clip_windows[TBBLUE_CLIP_WINDOW_SPRITES][3]);
					zxvision_print_string_defaults(&ventana,1,linea++,texto_buffer);


//                    //overwrite currently selected clip-window index value by "selection" graphics
//                    clip_select_x = clip_index_string_pos_x[tbblue_get_clip_window_sprites_index()];
//                    texto_buffer[clip_select_x+3] = 0;      // display only three digits in new colour
//					zxvision_print_string(&ventana,1+clip_select_x,linea++,
//                                          ESTILO_GUI_TINTA_NORMAL,ESTILO_GUI_PAPEL_SELECCIONADO,0,texto_buffer+clip_select_x);



					sprintf (texto_buffer,"ULA:     X=%3d-%3d Y=%3d-%3d",
					clip_windows[TBBLUE_CLIP_WINDOW_ULA][0],clip_windows[TBBLUE_CLIP_WINDOW_ULA][1],clip_windows[TBBLUE_CLIP_WINDOW_ULA][2],clip_windows[TBBLUE_CLIP_WINDOW_ULA][3]);
					zxvision_print_string_defaults(&ventana,1,linea++,texto_buffer);



//                    //overwrite currently selected clip-window index value by "selection" graphics
//                    clip_select_x = clip_index_string_pos_x[tbblue_get_clip_window_ula_index()];
//                    texto_buffer[clip_select_x+3] = 0;      // display only three digits in new colour
//					zxvision_print_string_defaults(&ventana,1+clip_select_x,linea++,
//                                          ESTILO_GUI_TINTA_NORMAL,ESTILO_GUI_PAPEL_SELECCIONADO,0,texto_buffer+clip_select_x);




					sprintf (texto_buffer,"Tilemap: X=%3d-%3d Y=%3d-%3d",
					clip_windows[TBBLUE_CLIP_WINDOW_TILEMAP][0]*2,clip_windows[TBBLUE_CLIP_WINDOW_TILEMAP][1]*2+1,clip_windows[TBBLUE_CLIP_WINDOW_TILEMAP][2],clip_windows[TBBLUE_CLIP_WINDOW_TILEMAP][3]);
					zxvision_print_string_defaults(&ventana,1,linea++,texto_buffer);



//                    //overwrite currently selected clip-window index value by "selection" graphics
//                    clip_select_x = clip_index_string_pos_x[tbblue_get_clip_window_tilemap_index()];
//                    texto_buffer[clip_select_x+3] = 0;      // display only three digits in new colour
//					zxvision_print_string(&ventana,1+clip_select_x,linea++,
//                                          ESTILO_GUI_TINTA_NORMAL,ESTILO_GUI_PAPEL_SELECCIONADO,0,texto_buffer+clip_select_x);


					linea++;
					sprintf (texto_buffer,"Offset Windows:");
					//menu_escribe_linea_opcion(linea++,-1,1,texto_buffer);	
					zxvision_print_string_defaults(&ventana,1,linea++,texto_buffer);

					sprintf (texto_buffer,"Layer2:     X=%4d  Y=%3d",tbblue_registers[22],tbblue_registers[23]);
					zxvision_print_string_defaults(&ventana,1,linea++,texto_buffer);


					sprintf (texto_buffer,"ULA/LoRes:  X=%4d  Y=%3d",tbblue_registers[50],tbblue_registers[51]);
					zxvision_print_string_defaults(&ventana,1,linea++,texto_buffer);

					//Offset X puede llegar hasta 1023. Por tanto 4 cifras. El resto X solo 3 cifras, pero los dejamos a 4 para que formato quede igual en pantalla
					sprintf (texto_buffer,"Tilemap:    X=%4d  Y=%3d",tbblue_registers[48]+256*(tbblue_registers[47]&3),tbblue_registers[49]);
					zxvision_print_string_defaults(&ventana,1,linea++,texto_buffer);					

				}



				zxvision_draw_window_contents(&ventana);


                }

				//Nota: No usamos zxvision_common_getkey_refresh porque necesitamos que el bucle se ejecute continuamente para poder 
				//refrescar contenido de ventana, dado que aqui no llamamos a menu_espera_tecla
				//(a no ser que este multitarea off)
				tecla=zxvision_common_getkey_refresh_noesperatecla();


				zxvision_handle_cursors_pgupdn(&ventana,tecla);


		} while (tecla!=2);

        cls_menu_overlay();
		zxvision_destroy_window(&ventana);


}



//int tsconf_spritenav_window_y=2;
//int tsconf_spritenav_window_alto=20;

#define TSCONF_SPRITENAV_WINDOW_X 0
#define TSCONF_SPRITENAV_WINDOW_Y 2
#define TSCONF_SPRITENAV_WINDOW_ANCHO 32
#define TSCONF_SPRITENAV_WINDOW_ALTO 20
//#define TSCONF_SPRITENAV_SPRITES_PER_WINDOW 8
//#define TSCONF_SPRITENAV_SPRITES_PER_WINDOW ((tsconf_spritenav_window_alto-4)/2)





//int menu_debug_tsconf_tbblue_spritenav_current_palette=0;
int menu_debug_tsconf_tbblue_spritenav_current_sprite=0;


zxvision_window *menu_debug_tsconf_tbblue_spritenav_draw_sprites_window;

int menu_debug_tsconf_tbblue_spritenav_get_total_sprites(void)
{
	int limite;

	limite=TSCONF_MAX_SPRITES; //85 sprites max

	if (MACHINE_IS_TBBLUE) limite=TBBLUE_MAX_SPRITES;

	return limite;
}

//Muestra lista de sprites
void menu_debug_tsconf_tbblue_spritenav_lista_sprites(void)
{

	char dumpmemoria[33];

	int linea_color;
	int limite;

	int linea=0;
	/*limite=TSCONF_MAX_SPRITES; //85 sprites max

	if (MACHINE_IS_TBBLUE) limite=TBBLUE_MAX_SPRITES;*/

	limite=menu_debug_tsconf_tbblue_spritenav_get_total_sprites();

	//z80_byte tbsprite_sprites[TBBLUE_MAX_SPRITES][4];
	/*
	1st: X position (bits 7-0).
2nd: Y position (0-255).
3rd: bits 7-4 is palette offset, bit 3 is X mirror, bit 2 is Y mirror, bit 1 is the rotate flag and bit 0 is X MSB.
4th: bit 7 is the visible flag, bit 6 is reserved, bits 5-0 is Name (pattern index, 0-63).
*/

	int current_sprite;


		/*for (linea_color=0;linea_color<TSCONF_SPRITENAV_SPRITES_PER_WINDOW &&
				menu_debug_tsconf_tbblue_spritenav_current_sprite+linea_color<limite;
				linea_color++) {*/
		for (linea_color=0;linea_color<limite;linea_color++) {					

			current_sprite=menu_debug_tsconf_tbblue_spritenav_current_sprite+linea_color;

			if (MACHINE_IS_TSCONF) {

			int offset=current_sprite*6;
			z80_byte sprite_r0h=tsconf_fmaps[0x200+offset+1];

			z80_byte sprite_leap=sprite_r0h&64;

			int sprite_act=sprite_r0h&32;
        	int y=tsconf_fmaps[0x200+offset]+256*(sprite_r0h&1);
	      	z80_byte ysize=8*(1+((sprite_r0h>>1)&7));
	               

        	z80_byte sprite_r1h=tsconf_fmaps[0x200+offset+3];
		    int x=tsconf_fmaps[0x200+offset+2]+256*(sprite_r1h&1);
			z80_byte xsize=8*(1+((sprite_r1h>>1)&7));

			z80_byte sprite_r2h=tsconf_fmaps[0x200+offset+5];
			z80_int tnum=(tsconf_fmaps[0x200+offset+4])+256*(sprite_r2h&15);
			    	//Tile Number for upper left corner. Bits 0-5 are X Position in Graphics Bitmap, bits 6-11 - Y Position.
			z80_int tnum_x=tnum & 63;
    		z80_int tnum_y=(tnum>>6)&63;

		    z80_byte spal=(sprite_r2h>>4)&15;

			z80_byte sprite_xf=sprite_r1h&128;
			z80_byte sprite_yf=sprite_r0h&128;

			sprintf (dumpmemoria,"%02d X: %3d Y: %3d (%2dX%2d)",current_sprite,x,y,xsize,ysize);
			//menu_escribe_linea_opcion(linea++,-1,1,dumpmemoria);
			zxvision_print_string_defaults(menu_debug_tsconf_tbblue_spritenav_draw_sprites_window,1,linea++,dumpmemoria);

			sprintf (dumpmemoria,"Tile:%2d,%2d %s %s %s %s P:%2d",tnum_x,tnum_y,
				(sprite_act ? "ACT" : "   "),(sprite_leap ? "LEAP": "    "),
				(sprite_xf ? "XF" : "  "),(sprite_yf ? "YF": "  "),
				spal );

			//menu_escribe_linea_opcion(linea++,-1,1,dumpmemoria);
			zxvision_print_string_defaults(menu_debug_tsconf_tbblue_spritenav_draw_sprites_window,1,linea++,dumpmemoria);
			}

			if (MACHINE_IS_TBBLUE) {
					//z80_byte tbsprite_sprites[TBBLUE_MAX_SPRITES][4];
	/*
	1st: X position (bits 7-0).
2nd: Y position (0-255).
3rd: bits 7-4 is palette offset, bit 3 is X mirror, bit 2 is Y mirror, bit 1 is the rotate flag and bit 0 is X MSB.
4th: bit 7 is the visible flag, bit 6 is reserved, bits 5-0 is Name (pattern index, 0-63).
*/

				z80_int x=tbsprite_sprites[current_sprite][0]; //
				z80_byte y=tbsprite_sprites[current_sprite][1];  //

				z80_byte byte_3=tbsprite_sprites[current_sprite][2];
				z80_byte paloff=byte_3 & 0xF0; //
				z80_byte mirror_x=byte_3 & 8; //
				z80_byte mirror_y=byte_3 & 4; //
				z80_byte rotate=byte_3 & 2; //
				z80_byte msb_x=byte_3 &1; //

				x +=msb_x*256;

				z80_byte byte_4=tbsprite_sprites[current_sprite][3];
				z80_byte visible=byte_4 & 128; //
				z80_byte pattern=byte_4 & 63; //

			sprintf (dumpmemoria,"%02d X: %3d Y: %3d %s %s %s",current_sprite,x,y,
					(mirror_x ? "MIRX" : "    "),(mirror_y ? "MIRY" : "    "),(rotate ? "ROT" : "   ")
			);
			//menu_escribe_linea_opcion(linea++,-1,1,dumpmemoria);
			zxvision_print_string_defaults(menu_debug_tsconf_tbblue_spritenav_draw_sprites_window,1,linea++,dumpmemoria);

			sprintf (dumpmemoria," Pattn: %2d Palof: %3d Vis: %s"
				,pattern,paloff, (visible ? "Yes" : "No ") );


				//menu_escribe_linea_opcion(linea++,-1,1,dumpmemoria);
				zxvision_print_string_defaults(menu_debug_tsconf_tbblue_spritenav_draw_sprites_window,1,linea++,dumpmemoria);

			}			

			


					
		}

	zxvision_draw_window_contents(menu_debug_tsconf_tbblue_spritenav_draw_sprites_window); 


}

void menu_debug_tsconf_tbblue_spritenav_draw_sprites(void)
{



				/*menu_speech_tecla_pulsada=1; //Si no, envia continuamente todo ese texto a speech


				//Mostrar lista sprites
				menu_debug_tsconf_tbblue_spritenav_lista_sprites();

				//Esto tiene que estar despues de escribir la lista de sprites, para que se refresque y se vea
				//Si estuviese antes, al mover el cursor hacia abajo dejándolo pulsado, el texto no se vería hasta que no se soltase la tecla
				normal_overlay_texto_menu();*/


				menu_speech_tecla_pulsada=1; //Si no, envia continuamente todo ese texto a speech
				normal_overlay_texto_menu();
				menu_debug_tsconf_tbblue_spritenav_lista_sprites();



}



void menu_debug_tsconf_tbblue_spritenav(MENU_ITEM_PARAMETERS)
{
	menu_espera_no_tecla();
	menu_reset_counters_tecla_repeticion();

	
	zxvision_window ventana;

	zxvision_new_window(&ventana,TSCONF_SPRITENAV_WINDOW_X,TSCONF_SPRITENAV_WINDOW_Y,TSCONF_SPRITENAV_WINDOW_ANCHO,TSCONF_SPRITENAV_WINDOW_ALTO,
							TSCONF_SPRITENAV_WINDOW_ANCHO-1,menu_debug_tsconf_tbblue_spritenav_get_total_sprites()*2,"Sprite navigator");

	zxvision_draw_window(&ventana);		

    set_menu_overlay_function(menu_debug_tsconf_tbblue_spritenav_draw_sprites);

	menu_debug_tsconf_tbblue_spritenav_draw_sprites_window=&ventana; //Decimos que el overlay lo hace sobre la ventana que tenemos aqui


	z80_byte tecla;

		//Si no esta multitarea, hacer un refresco inicial para que aparezca el contenido de la ventana sin tener que pulsar una tecla
		//dado que luego funciona como overlay, el overlay se aplica despues de hacer el render
		//esto solo es necesario para ventanas que usan overlay
	    if (!menu_multitarea) {
			//printf ("refresca pantalla inicial\n");
			menu_refresca_pantalla();
		}			

	
    do {
    	menu_speech_tecla_pulsada=0; //Que envie a speech
   		tecla=zxvision_common_getkey_refresh();
		zxvision_handle_cursors_pgupdn(&ventana,tecla);
	} while (tecla!=2);  

	//restauramos modo normal de texto de menu
    set_menu_overlay_function(normal_overlay_texto_menu);		

    cls_menu_overlay();
	zxvision_destroy_window(&ventana);
}








#define TSCONF_TILENAV_WINDOW_X 0
#define TSCONF_TILENAV_WINDOW_Y 0
#define TSCONF_TILENAV_WINDOW_ANCHO 32
#define TSCONF_TILENAV_WINDOW_ALTO 24
#define TSCONF_TILENAV_TILES_VERT_PER_WINDOW 64
#define TSCONF_TILENAV_TILES_HORIZ_PER_WINDOW 64



//int menu_debug_tsconf_tbblue_tilenav_current_palette=0;
//int menu_debug_tsconf_tbblue_tilenav_current_tile=0;

int menu_debug_tsconf_tbblue_tilenav_current_tilelayer=0;

z80_bit menu_debug_tsconf_tbblue_tilenav_showmap={0};

zxvision_window *menu_debug_tsconf_tbblue_tilenav_lista_tiles_window;


#define DEBUG_TSCONF_TILENAV_MAX_TILES (64*64)
//#define DEBUG_TBBLUE_TILENAV_MAX_TILES_8032 (80*32)
//#define DEBUG_TBBLUE_TILENAV_MAX_TILES_4032 (40*32)


char menu_debug_tsconf_tbblue_tiles_retorna_visualchar(int tnum)
{
	//Hacer un conjunto de 64 caracteres. Mismo set de caracteres que para Base64. Por que? Por que si :)
			   //0123456789012345678901234567890123456789012345678901234567890123
	char *caracter_list="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

	int index=tnum % 64;
	return caracter_list[index];
}

int menu_debug_tsconf_tbblue_tilenav_total_vert(void)
{

	int limite_vertical;

	if (MACHINE_IS_TSCONF) {
		limite_vertical=DEBUG_TSCONF_TILENAV_MAX_TILES;
		if (menu_debug_tsconf_tbblue_tilenav_showmap.v) limite_vertical=TSCONF_TILENAV_TILES_VERT_PER_WINDOW;	
	}

	else  { //TBBLUE
		limite_vertical=tbblue_get_tilemap_width()*32;

		if (menu_debug_tsconf_tbblue_tilenav_showmap.v) limite_vertical=32;	
	}

	return limite_vertical;
}

//Muestra lista de tiles
void menu_debug_tsconf_tbblue_tilenav_lista_tiles(void)
{

	//Suficientemente grande para almacenar regla superior en modo visual
	char dumpmemoria[84]; //80 + 3 espacios izquierda + 0 final

	
	//int limite;

	int linea=0;
	//limite=DEBUG_TSCONF_TILENAV_MAX_TILES;

	int current_tile;

	z80_byte *puntero_tilemap;
	z80_byte *puntero_tilemap_orig;

	if (MACHINE_IS_TSCONF) {
		puntero_tilemap=tsconf_ram_mem_table[0]+tsconf_return_tilemappage();
	}

	else {  //TBBLUE
		//puntero_tilemap=tbblue_ram_mem_table[5]+tbblue_get_offset_start_tilemap();
			//Siempre saldra de ram 5
		puntero_tilemap=tbblue_ram_memory_pages[5*2]+(256*tbblue_get_offset_start_tilemap());	
		//printf ("%XH\n",tbblue_get_offset_start_tilemap() );

	}

	z80_byte tbblue_tilemap_control;
	int tilemap_width;


	int tbblue_bytes_per_tile=2;

	if (MACHINE_IS_TBBLUE) {
					/*
					(R/W) 0x6B (107) => Tilemap Control
  bit 7    = 1 to enable the tilemap
  bit 6    = 0 for 40x32, 1 for 80x32
  bit 5    = 1 to eliminate the attribute entry in the tilemap
  bit 4    = palette select
  bits 3-0 = Reserved set to 0
					*/
					tbblue_tilemap_control=tbblue_registers[107];

					if (tbblue_tilemap_control&32) tbblue_bytes_per_tile=1;

					tilemap_width=tbblue_get_tilemap_width();

	}

	puntero_tilemap_orig=puntero_tilemap;

	int limite_vertical=menu_debug_tsconf_tbblue_tilenav_total_vert();


	int offset_vertical=0;

	if (menu_debug_tsconf_tbblue_tilenav_showmap.v) {
		if (MACHINE_IS_TSCONF) {
				  //0123456789012345678901234567890123456789012345678901234567890123
		strcpy(dumpmemoria,"   0    5    10   15   20   25   30   35   40   45   50   55   60  ");
		}

		else { //TBBLUE
			if (tilemap_width==40) {
				             //0123456789012345678901234567890123456789012345678901234567890123
		strcpy(dumpmemoria,"   0    5    10   15   20   25   30   35   ");
			}
			else {
				             //01234567890123456789012345678901234567890123456789012345678901234567890123456789
		strcpy(dumpmemoria,"   0    5    10   15   20   25   30   35   40   45   50   55   60   65   70   75   ");
			}

		}

		//Indicar codigo 0 de final
		//dumpmemoria[current_tile_x+TSCONF_TILENAV_TILES_HORIZ_PER_WINDOW+3]=0;  //3 espacios al inicio

		//menu_escribe_linea_opcion(linea++,-1,1,&dumpmemoria[current_tile_x]); //Mostrar regla superior
		zxvision_print_string_defaults(menu_debug_tsconf_tbblue_tilenav_lista_tiles_window,1,0,dumpmemoria);
	}
	else {
		//Aumentarlo en cuanto al offset que estamos (si modo lista)

		int offset_y=menu_debug_tsconf_tbblue_tilenav_lista_tiles_window->offset_y;
		

		offset_vertical=offset_y/2;
		linea=offset_vertical*2;



		limite_vertical=offset_vertical+((24-2)/2)+1; //El maximo que cabe en pantalla, +1 para cuando se baja 1 posicion con cursor

	}

	//linea destino es +3, pues las tres primeras son de leyenda
	linea +=3;	



		for (;offset_vertical<limite_vertical;offset_vertical++) {

			int repetir_ancho=1;
			int mapa_tile_x=3;
			if (menu_debug_tsconf_tbblue_tilenav_showmap.v==0) {
				//Modo lista tiles
				current_tile=offset_vertical;
			}

			else {
				//Modo mapa tiles
				if (MACHINE_IS_TSCONF) {
					current_tile=offset_vertical*64;
					repetir_ancho=TSCONF_TILENAV_TILES_HORIZ_PER_WINDOW;

					//poner regla vertical
					int linea_tile=current_tile/64;
					if ( (linea_tile%5)==0) sprintf (dumpmemoria,"%2d ",linea_tile);
					else sprintf (dumpmemoria,"   ");
				}

				else { //TBBLUE
					current_tile=offset_vertical*tilemap_width;
					repetir_ancho=tilemap_width;

					//poner regla vertical
					int linea_tile=current_tile/tilemap_width;
					if ( (linea_tile%5)==0) sprintf (dumpmemoria,"%2d ",linea_tile);
					else sprintf (dumpmemoria,"   ");				
				}
			}

			//printf ("linea: %3d current tile: %10d puntero: %10d\n",linea_color,current_tile,puntero_tilemap-tsconf_ram_mem_table[0]-tsconf_return_tilemappage()	);

			do {
				if (MACHINE_IS_TSCONF) {
					int y=current_tile/64;
					int x=current_tile%64; 

					//printf ("x: %d y: %d\n",x,y);
				

					int offset=(256*y)+(x*2);

					offset+=menu_debug_tsconf_tbblue_tilenav_current_tilelayer*128;

					int tnum=puntero_tilemap[offset]+256*(puntero_tilemap[offset+1]&0xF);

					//printf ("Current tile: %d  x: %d y: %d  tnum: %d\n",current_tile,x,y,tnum);

					z80_byte tnum_x=tnum&63;
					z80_byte tnum_y=(tnum>>6)&63;

		    		z80_byte tpal=(puntero_tilemap[offset+1]>>4)&3;

					z80_byte tile_xf=puntero_tilemap[offset+1]&64;
					z80_byte tile_yf=puntero_tilemap[offset+1]&128;

					if (menu_debug_tsconf_tbblue_tilenav_showmap.v==0) {
						//Modo lista tiles
						sprintf (dumpmemoria,"X: %3d Y: %3d                   ",x,y);

						zxvision_print_string_defaults(menu_debug_tsconf_tbblue_tilenav_lista_tiles_window,1,linea++,dumpmemoria);

						sprintf (dumpmemoria," Tile: %2d,%2d %s %s P:%2d",tnum_x,tnum_y,
							(tile_xf ? "XF" : "  "),(tile_yf ? "YF": "  "),
							tpal );

						zxvision_print_string_defaults(menu_debug_tsconf_tbblue_tilenav_lista_tiles_window,1,linea++,dumpmemoria);
					}
					else {
						//Modo mapa tiles
						z80_byte caracter_final;

						if (tnum==0) {
							caracter_final=' '; 
						}
						else {
							caracter_final=menu_debug_tsconf_tbblue_tiles_retorna_visualchar(tnum);
						}

						dumpmemoria[mapa_tile_x++]=caracter_final;
					}
				}

				if (MACHINE_IS_TBBLUE) {

					int y=current_tile/tilemap_width;
					int x=current_tile%tilemap_width; 

					int offset=(tilemap_width*tbblue_bytes_per_tile*y)+(x*tbblue_bytes_per_tile);
					/*
					 bits 15-12 : palette offset
  bit     11 : x mirror
  bit     10 : y mirror
  bit      9 : rotate
  bit      8 : ULA over tilemap (if the ula is disabled, bit 8 of tile number)
  bits   7-0 : tile number
					*/

					int xmirror,ymirror,rotate;
					z80_byte tpal;

					z80_byte byte_first;
					z80_byte byte_second;

					byte_first=puntero_tilemap[offset];
					byte_second=puntero_tilemap[offset+1];					

					int tnum=byte_first;
					int ula_over_tilemap;

					z80_byte tbblue_default_tilemap_attr=tbblue_registers[108];

					if (tbblue_bytes_per_tile==1) {
						/*
						(R/W) 0x6C (108) => Default Tilemap Attribute
  bits 7-4 = Palette Offset
  bit 3    = X mirror
  bit 2    = Y mirror
  bit 1    = Rotate
  bit 0    = ULA over tilemap
             (bit 8 of tile id if the ULA is disabled)	
			 			*/
					 	tpal=(tbblue_default_tilemap_attr>>4)&15;
						xmirror=(tbblue_default_tilemap_attr>>3)&1;
						ymirror=(tbblue_default_tilemap_attr>>2)&1;
						rotate=(tbblue_default_tilemap_attr>>1)&1;

						if (tbblue_if_ula_is_enabled() ) {
						/*
						108
						  bit 0    = ULA over tilemap
             (bit 8 of tile id if the ULA is disabled)
						*/							
							ula_over_tilemap=tbblue_default_tilemap_attr &1;
						}

						else {
							tnum |=(tbblue_default_tilemap_attr&1)<<8; // bit      8 : ULA over tilemap (if the ula is disabled, bit 8 of tile number)
						}
						
					}

					else {
						/*
							
					 bits 15-12 : palette offset
  bit     11 : x mirror
  bit     10 : y mirror
  bit      9 : rotate
  bit      8 : ULA over tilemap (if the ula is disabled, bit 8 of tile number)
					*/	
					 	tpal=(byte_second>>4)&15;
						xmirror=(byte_second>>3)&1;
						ymirror=(byte_second>>2)&1;
						rotate=(byte_second>>1)&1;
						//ula_over_tilemap=byte_second &1;

					if (tbblue_if_ula_is_enabled() ) {
						/*
						  bit      8 : ULA over tilemap (if the ula is disabled, bit 8 of tile number)
						*/							
							ula_over_tilemap=byte_second &1;
						}

						else {
							tnum |=(byte_second&1)<<8; // bit      8 : ULA over tilemap (if the ula is disabled, bit 8 of tile number)
						}


					}

					//printf ("tnum: %d\n",tnum);


					if (menu_debug_tsconf_tbblue_tilenav_showmap.v==0) {
						//Modo lista tiles
						sprintf (dumpmemoria,"X: %3d Y: %3d                   ",x,y);

						zxvision_print_string_defaults(menu_debug_tsconf_tbblue_tilenav_lista_tiles_window,1,linea++,dumpmemoria);

						sprintf (dumpmemoria," Tile: %3d %s %s %s %s P:%2d ",tnum,
							(xmirror ? "MX" : "  "),(ymirror ? "MY": "  "),
							(rotate ? "R" : " "),(ula_over_tilemap ? "U": " "),
							tpal );

						zxvision_print_string_defaults(menu_debug_tsconf_tbblue_tilenav_lista_tiles_window,1,linea++,dumpmemoria);
					}
					else {
						//Modo mapa tiles
						int caracter_final;

						if (tnum==0) {
							caracter_final=' '; 
						}
						else {
							caracter_final=menu_debug_tsconf_tbblue_tiles_retorna_visualchar(tnum);
						}

						dumpmemoria[mapa_tile_x++]=caracter_final;
					}					


				}

				current_tile++;

				repetir_ancho--;
			} while (repetir_ancho);

			if (menu_debug_tsconf_tbblue_tilenav_showmap.v) {
				zxvision_print_string_defaults(menu_debug_tsconf_tbblue_tilenav_lista_tiles_window,1,linea++,dumpmemoria);
				puntero_tilemap=puntero_tilemap_orig;
			}
					
		}



	//return linea;

	zxvision_draw_window_contents(menu_debug_tsconf_tbblue_tilenav_lista_tiles_window); 
}

void menu_debug_tsconf_tbblue_tilenav_draw_tiles(void)
{
/*
				menu_speech_tecla_pulsada=1; //Si no, envia continuamente todo ese texto a speech


				//Mostrar lista tiles
				menu_debug_tsconf_tbblue_tilenav_lista_tiles();

				//Esto tiene que estar despues de escribir la lista de tiles, para que se refresque y se vea
				//Si estuviese antes, al mover el cursor hacia abajo dejándolo pulsado, el texto no se vería hasta que no se soltase la tecla
				normal_overlay_texto_menu();
				*/



				menu_speech_tecla_pulsada=1; //Si no, envia continuamente todo ese texto a speech
				normal_overlay_texto_menu();
				menu_debug_tsconf_tbblue_tilenav_lista_tiles();				

}



void menu_debug_tsconf_tbblue_tilenav_new_window(zxvision_window *ventana)
{

		char titulo[33];
		
		char linea_leyenda[64];
		sprintf (titulo,"Tile Navigator");

        //Forzar a mostrar atajos
        z80_bit antes_menu_writing_inverse_color;
        antes_menu_writing_inverse_color.v=menu_writing_inverse_color.v;
        menu_writing_inverse_color.v=1;		

		int total_height=menu_debug_tsconf_tbblue_tilenav_total_vert();
		int total_width=31;

		char texto_layer[32];

		//En caso de tbblue, solo hay una capa
		if (MACHINE_IS_TBBLUE) texto_layer[0]=0;

		else sprintf (texto_layer,"~~Layer %d",menu_debug_tsconf_tbblue_tilenav_current_tilelayer);

		if (menu_debug_tsconf_tbblue_tilenav_showmap.v) {
			sprintf (linea_leyenda,"~~Mode: Visual %s",texto_layer);

			if (MACHINE_IS_TSCONF) {
			total_width=TSCONF_TILENAV_TILES_HORIZ_PER_WINDOW+4;
			}
			else {
				//TBBLUE
				total_width=tbblue_get_tilemap_width()+4;
			}

		}

		else {
			sprintf (linea_leyenda,"~~Mode: List %s",texto_layer);
			total_height*=2;
		}

		//tres mas para ubicar las lineas de leyenda
		total_height+=3;

		zxvision_new_window(ventana,TSCONF_TILENAV_WINDOW_X,TSCONF_TILENAV_WINDOW_Y,TSCONF_TILENAV_WINDOW_ANCHO,TSCONF_TILENAV_WINDOW_ALTO,
							total_width,total_height,titulo);


		//Establecer leyenda en la parte de abajo
		ventana->lower_margin=2;
		//Texto sera el de la primera linea
		ventana->upper_margin=1;


		
		//Leyenda inferior
		//zxvision_print_string_defaults_fillspc(ventana,1,1,"-----");
		zxvision_print_string_defaults_fillspc(ventana,1,2,linea_leyenda);

		zxvision_draw_window(ventana);	

        //Restaurar comportamiento atajos
        menu_writing_inverse_color.v=antes_menu_writing_inverse_color.v;
		//Nota: los atajos se "pintan" en la memoria de la ventana ya con el color inverso
		//por tanto con meter al principio la variable de inverse_color es suficiente
		//y no hay que activar inverse color cada vez que se redibuja ventana,
		//pues al redibujar ventana está leyendo el contenido de la memoria de la ventana, y ahí ya está con color inverso		

}

void menu_debug_tsconf_tbblue_tilenav(MENU_ITEM_PARAMETERS)
{

	menu_espera_no_tecla();
	menu_reset_counters_tecla_repeticion();

	
		zxvision_window ventana;


		menu_debug_tsconf_tbblue_tilenav_new_window(&ventana);



        set_menu_overlay_function(menu_debug_tsconf_tbblue_tilenav_draw_tiles);


		menu_debug_tsconf_tbblue_tilenav_lista_tiles_window=&ventana; //Decimos que el overlay lo hace sobre la ventana que tenemos aqui


		z80_byte tecla;

		//Si no esta multitarea, hacer un refresco inicial para que aparezca el contenido de la ventana sin tener que pulsar una tecla
		//dado que luego funciona como overlay, el overlay se aplica despues de hacer el render
		//esto solo es necesario para ventanas que usan overlay
	    if (!menu_multitarea) {
			//printf ("refresca pantalla inicial\n");
			menu_refresca_pantalla();
		}				


	do {
    	menu_speech_tecla_pulsada=0; //Que envie a speech


			

		tecla=zxvision_common_getkey_refresh();				

        
				switch (tecla) {

					case 'l':
						//En caso de tbblue, hay una sola capa
						if (!MACHINE_IS_TBBLUE) {					
							zxvision_destroy_window(&ventana);	
							menu_debug_tsconf_tbblue_tilenav_current_tilelayer ^=1;
							menu_debug_tsconf_tbblue_tilenav_new_window(&ventana);
						}
					break;

					case 'm':

						zxvision_destroy_window(&ventana);		
						menu_debug_tsconf_tbblue_tilenav_showmap.v ^=1;
						menu_debug_tsconf_tbblue_tilenav_new_window(&ventana);

					break;


					default:
						zxvision_handle_cursors_pgupdn(&ventana,tecla);
					break;
				}		

		


	} while (tecla!=2); 

	//restauramos modo normal de texto de menu
    set_menu_overlay_function(normal_overlay_texto_menu);		

    cls_menu_overlay();
	zxvision_destroy_window(&ventana);		


	
    

}




#define SOUND_WAVE_X 1
#define SOUND_WAVE_Y 3
#define SOUND_WAVE_ANCHO 30
#define SOUND_WAVE_ALTO 15

int menu_sound_wave_llena=1;
int menu_audio_draw_sound_wave_ycentro;

char menu_audio_draw_sound_wave_valor_medio,menu_audio_draw_sound_wave_valor_max,menu_audio_draw_sound_wave_valor_min;
int menu_audio_draw_sound_wave_frecuencia_aproximada;

int menu_audio_draw_sound_wave_volumen=0;
int menu_audio_draw_sound_wave_volumen_escalado=0;


//Usado dentro del overlay de waveform, para mostrar dos veces por segundo el texto que average, etc
int menu_waveform_valor_contador_segundo_anterior;

int menu_waveform_previous_volume=0;



zxvision_window *menu_audio_draw_sound_wave_window;

void menu_audio_draw_sound_wave(void)
{

	if (!zxvision_drawing_in_background) normal_overlay_texto_menu();

	//workaround_pentagon_clear_putpixel_cache();

				char buffer_texto_medio[40]; //32+3+margen de posible color rojo del maximo

	menu_speech_tecla_pulsada=1; //Si no, envia continuamente todo ese texto a speech

	//esto hara ejecutar esto 2 veces por segundo
	if ( ((contador_segundo%500) == 0 && menu_waveform_valor_contador_segundo_anterior!=contador_segundo) || menu_multitarea==0) {

		menu_waveform_valor_contador_segundo_anterior=contador_segundo;
		//printf ("Refrescando. contador_segundo=%d\n",contador_segundo);

		//menu_speech_tecla_pulsada=1; //Si no, envia continuamente todo ese texto a speech

			//Average, min, max    

			sprintf (buffer_texto_medio,"Av.: %d Min: %d Max: %d",
			menu_audio_draw_sound_wave_valor_medio,menu_audio_draw_sound_wave_valor_min,menu_audio_draw_sound_wave_valor_max);


			//menu_escribe_linea_opcion(1,-1,1,buffer_texto_medio);
			zxvision_print_string_defaults_fillspc(menu_audio_draw_sound_wave_window,1,1,buffer_texto_medio);


	

			//Hacer decaer el volumen
			//if (menu_waveform_previous_volume>menu_audio_draw_sound_wave_volumen_escalado) menu_waveform_previous_volume--;
			menu_waveform_previous_volume=menu_decae_dec_valor_volumen(menu_waveform_previous_volume,menu_audio_draw_sound_wave_volumen_escalado);


			//Frecuency
			sprintf (buffer_texto_medio,"Average freq: %d Hz (%s)",
				menu_audio_draw_sound_wave_frecuencia_aproximada,get_note_name(menu_audio_draw_sound_wave_frecuencia_aproximada));
			//menu_escribe_linea_opcion(3,-1,1,buffer_texto_medio);
			zxvision_print_string_defaults_fillspc(menu_audio_draw_sound_wave_window,1,3,buffer_texto_medio);

			//printf ("menu_speech_tecla_pulsada: %d\n",menu_speech_tecla_pulsada);
	}







	int ancho=(SOUND_WAVE_ANCHO-2);
	int alto=(SOUND_WAVE_ALTO-4);
	//int xorigen=(SOUND_WAVE_X+1);
	//int yorigen=(SOUND_WAVE_Y+4);

	int xorigen=1;
	int yorigen=4;


	if (si_complete_video_driver() ) {
        	ancho *=menu_char_width;
	        alto *=8;
        	xorigen *=menu_char_width;
	        yorigen *=8;
	}


	//int ycentro=yorigen+alto/2;
	menu_audio_draw_sound_wave_ycentro=yorigen+alto/2;

	int x,y,lasty;


	//Para drivers de texto, borrar zona

	if (!si_complete_video_driver() ) {
	        for (x=xorigen;x<xorigen+ancho;x++) {
        	        for (y=yorigen;y<yorigen+alto;y++) {
				//putchar_menu_overlay(x,y,' ',0,7);
				//putchar_menu_overlay(x,y,' ',ESTILO_GUI_TINTA_NORMAL,ESTILO_GUI_PAPEL_NORMAL);
				//putchar_menu_overlay(x,y,' ',ESTILO_GUI_COLOR_WAVEFORM,ESTILO_GUI_PAPEL_NORMAL);

				zxvision_print_char_simple(menu_audio_draw_sound_wave_window,x,y,ESTILO_GUI_TINTA_NORMAL,ESTILO_GUI_PAPEL_NORMAL,0,' ');
	                }
        	}
	}


	

	audiobuffer_stats audiostats;
	audio_get_audiobuffer_stats(&audiostats);


	menu_audio_draw_sound_wave_valor_max=audiostats.maximo;
	menu_audio_draw_sound_wave_valor_min=audiostats.minimo;
	menu_audio_draw_sound_wave_frecuencia_aproximada=audiostats.frecuencia;
	menu_audio_draw_sound_wave_volumen=audiostats.volumen;
	menu_audio_draw_sound_wave_volumen_escalado=audiostats.volumen_escalado;

	int audiomedio=audiostats.medio;
	menu_audio_draw_sound_wave_valor_medio=audiomedio;
        audiomedio=audiomedio*alto/256;

        //Lo situamos en el centro. Negativo hacia abajo (Y positiva)
        audiomedio=menu_audio_draw_sound_wave_ycentro-audiomedio;

	int puntero_audio=0;
	char valor_audio;
	for (x=xorigen;x<xorigen+ancho;x++) {

		//Obtenemos valor medio de audio
		int valor_medio=0;

		//Calcular cuantos valores representa un pixel, teniendo en cuenta maximo buffer
		const int max_valores=AUDIO_BUFFER_SIZE/ancho;

		int valores=max_valores;
		for (;valores>0;valores--,puntero_audio++) {
			if (puntero_audio>=AUDIO_BUFFER_SIZE) {
				//por si el calculo no es correcto, salir.
				//esto no deberia suceder ya que el calculo de max_valores se hace en base al maximo
				cpu_panic("menu_audio_draw_sound_wave: pointer beyond AUDIO_BUFFER_SIZE");
			}

			//stereo 
			//if (audio_driver_accepts_stereo.v) {
				int suma_canales=audio_buffer[puntero_audio*2]+audio_buffer[(puntero_audio*2)+1];
				suma_canales /=2;
				valor_medio=valor_medio+suma_canales;
			//}

			//else valor_medio=valor_medio+audio_buffer[puntero_audio];


		}

		valor_medio=valor_medio/max_valores;

		valor_audio=valor_medio;

		//Lo escalamos a maximo alto

		y=valor_audio;
		y=valor_audio*alto/256;

		//Lo situamos en el centro. Negativo hacia abajo (Y positiva)
		y=menu_audio_draw_sound_wave_ycentro-y;


		//unimos valor anterior con actual con una linea vertical
		if (x!=xorigen) {
			if (si_complete_video_driver() ) {

				//Onda no llena
				if (!menu_sound_wave_llena) menu_linea_zxvision(menu_audio_draw_sound_wave_window,x,lasty,y,ESTILO_GUI_COLOR_WAVEFORM);

        			//dibujar la onda "llena", es decir, siempre contar desde centro
			        //el centro de la y de la onda es variable... se saca valor medio de todos los valores mostrados en pantalla

				//Onda llena
				else menu_linea_zxvision(menu_audio_draw_sound_wave_window,x,audiomedio,y,ESTILO_GUI_COLOR_WAVEFORM);



			}
		}

		lasty=y;

		//dibujamos valor actual
		if (si_complete_video_driver() ) {
			//menu_scr_putpixel(x,y,ESTILO_GUI_COLOR_WAVEFORM);
			zxvision_putpixel(menu_audio_draw_sound_wave_window,x,y,ESTILO_GUI_COLOR_WAVEFORM);
		}

		else {
			//putchar_menu_overlay(SOUND_WAVE_X+x,SOUND_WAVE_Y+y,'#',ESTILO_GUI_COLOR_WAVEFORM,ESTILO_GUI_PAPEL_NORMAL);
			zxvision_print_char_simple(menu_audio_draw_sound_wave_window,x,y,ESTILO_GUI_TINTA_NORMAL,ESTILO_GUI_PAPEL_NORMAL,0,'#');
		}


	}

	//printf ("%d ",puntero_audio);


	//Volume. Mostrarlo siempre, no solo dos veces por segundo, para que se actualice mas frecuentemente
	//if (menu_waveform_previous_volume<menu_audio_draw_sound_wave_volumen_escalado) menu_waveform_previous_volume=menu_audio_draw_sound_wave_volumen_escalado;
	menu_waveform_previous_volume=menu_decae_ajusta_valor_volumen(menu_waveform_previous_volume,menu_audio_draw_sound_wave_volumen_escalado);

	char texto_volumen[32]; 
    menu_string_volumen(texto_volumen,menu_audio_draw_sound_wave_volumen_escalado,menu_waveform_previous_volume);
                                                                //"Volume C: %s"

	sprintf (buffer_texto_medio,"Volume: %3d %s",menu_audio_draw_sound_wave_volumen,texto_volumen);
	//menu_escribe_linea_opcion(2,-1,1,buffer_texto_medio);
	zxvision_print_string_defaults_fillspc(menu_audio_draw_sound_wave_window,1,2,buffer_texto_medio);


	zxvision_draw_window_contents(menu_audio_draw_sound_wave_window); 

}




void menu_audio_new_waveform_shape(MENU_ITEM_PARAMETERS)
{
	menu_sound_wave_llena ^=1;
}



void menu_audio_new_waveform(MENU_ITEM_PARAMETERS)
{

 	menu_espera_no_tecla();
	menu_reset_counters_tecla_repeticion();		

	zxvision_window ventana;

	zxvision_new_window(&ventana,SOUND_WAVE_X,SOUND_WAVE_Y-2,SOUND_WAVE_ANCHO,SOUND_WAVE_ALTO+4,
							SOUND_WAVE_ANCHO-1,SOUND_WAVE_ALTO+4-2,"Waveform");
	zxvision_draw_window(&ventana);		

    
    //Cambiamos funcion overlay de texto de menu
    //Se establece a la de funcion de audio waveform
	set_menu_overlay_function(menu_audio_draw_sound_wave);

	menu_audio_draw_sound_wave_window=&ventana; //Decimos que el overlay lo hace sobre la ventana que tenemos aqui

	menu_item *array_menu_audio_new_waveform;
        menu_item item_seleccionado;
        int retorno_menu;
        do {


		menu_add_item_menu_inicial_format(&array_menu_audio_new_waveform,MENU_OPCION_NORMAL,menu_audio_new_waveform_shape,NULL,"Change wave ~~Shape");
        menu_add_item_menu_shortcut(array_menu_audio_new_waveform,'s');

        //Evito tooltips en los menus tabulados que tienen overlay porque al salir el tooltip detiene el overlay
        //menu_add_item_menu_tooltip(array_menu_audio_new_waveform,"Change wave Shape");
        menu_add_item_menu_ayuda(array_menu_audio_new_waveform,"Change wave Shape: simple line or vertical fill");
						
		menu_add_item_menu_tabulado(array_menu_audio_new_waveform,1,0);


		//Nombre de ventana solo aparece en el caso de stdout
    	retorno_menu=menu_dibuja_menu(&audio_new_waveform_opcion_seleccionada,&item_seleccionado,array_menu_audio_new_waveform,"Waveform" );


		//En caso de menus tabulados, es responsabilidad de este de borrar la ventana
		cls_menu_overlay();
        if ((item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu>=0) {
        	//llamamos por valor de funcion
            if (item_seleccionado.menu_funcion!=NULL) {
                //printf ("actuamos por funcion\n");
                item_seleccionado.menu_funcion(item_seleccionado.valor_opcion);
		//En caso de menus tabulados, es responsabilidad de este de borrar la ventana
                
            }
        }

    } while ( (item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu!=MENU_RETORNO_ESC && !salir_todos_menus);


	//restauramos modo normal de texto de menu
    set_menu_overlay_function(normal_overlay_texto_menu);


    cls_menu_overlay();

	//En caso de menus tabulados, es responsabilidad de este de liberar ventana
	zxvision_destroy_window(&ventana);

}


zxvision_window *menu_debug_draw_visualmem_window;



#ifdef EMULATE_VISUALMEM

#define VISUALMEM_MAX_ALTO 24
#define VISUALMEM_MAX_ANCHO 32

//int visualmem_ancho_variable=VISUALMEM_MAX_ANCHO-2;
//int visualmem_alto_variable=VISUALMEM_MAX_ALTO-2;

#define visualmem_ancho_variable (menu_debug_draw_visualmem_window->visible_width-1)
#define visualmem_alto_variable (menu_debug_draw_visualmem_window->visible_height-1)

#define VISUALMEM_MIN_X 0
#define VISUALMEM_MIN_Y 0

#define VISUALMEM_DEFAULT_X (VISUALMEM_MIN_X+1)
int visualmem_x_variable=VISUALMEM_DEFAULT_X;

#define VISUALMEM_DEFAULT_Y (VISUALMEM_MIN_Y+1)
int visualmem_y_variable=VISUALMEM_DEFAULT_Y;

#define VISUALMEM_ANCHO (visualmem_ancho_variable)
#define VISUALMEM_ALTO (visualmem_alto_variable)

#define VISUALMEM_DEFAULT_WINDOW_ANCHO (VISUALMEM_MAX_ANCHO-2)
#define VISUALMEM_DEFAULT_WINDOW_ALTO (VISUALMEM_MAX_ALTO-2)

//0=vemos visualmem write
//1=vemos visualmem read
//2=vemos visualmem opcode
//3=vemos todos a la vez
int menu_visualmem_donde=0;


int visualmem_bright_multiplier=10;


void menu_debug_draw_visualmem(void)
{

        normal_overlay_texto_menu();

		//workaround_pentagon_clear_putpixel_cache();


        int ancho=(VISUALMEM_ANCHO-2);
        int alto=(VISUALMEM_ALTO-4);

		if (ancho<1 || alto<1) return;

        //int xorigen=(visualmem_x_variable+1);
        //int yorigen=(visualmem_y_variable+5);
        int xorigen=1;
        int yorigen=3;


        if (si_complete_video_driver() ) {
                ancho *=menu_char_width;
                alto *=8;
                xorigen *=menu_char_width;
                yorigen *=8;
        }


	int tamanyo_total=ancho*alto;

        int x,y;


        int inicio_puntero_membuffer,final_puntero_membuffer;

	//Por defecto
	inicio_puntero_membuffer=16384;
	final_puntero_membuffer=65536;

	//printf ("ancho: %d alto: %d\n",ancho,alto);

	//En spectrum 16kb
	if (MACHINE_IS_SPECTRUM_16) {
		//printf ("spec 16kb\n");
		final_puntero_membuffer=32768;
	}

	if (MACHINE_IS_Z88) {
		        inicio_puntero_membuffer=0;
	}

	//En Inves, mostramos modificaciones a la RAM baja
	if (MACHINE_IS_INVES) {
                        inicio_puntero_membuffer=0;
        }



	//En zx80, zx81 mostrar desde 8192 por si tenemos expansion packs
	if (MACHINE_IS_ZX8081) {
		//por defecto
		//printf ("ramtop_zx8081: %d\n",ramtop_zx8081);
		final_puntero_membuffer=ramtop_zx8081+1;

		if (ram_in_8192.v) {
			//printf ("memoria en 8192\n");
			inicio_puntero_membuffer=8192;
		}

        	if (ram_in_32768.v) {
			//printf ("memoria en 32768\n");
			final_puntero_membuffer=49152;
		}

	        if (ram_in_49152.v) {
			//printf ("memoria en 49152\n");
			final_puntero_membuffer=65536;
		}

	}

        //En Jupiter Ace, desde 8192
        if (MACHINE_IS_ACE) {
                //por defecto
                final_puntero_membuffer=ramtop_ace+1;
                inicio_puntero_membuffer=8192;

        }


	//En CPC, desde 0
	if (MACHINE_IS_CPC) {
		inicio_puntero_membuffer=0;
	}

	if (MACHINE_IS_SAM) {
                inicio_puntero_membuffer=0;
        }



	//En modos de RAM en ROM de +2a en puntero apuntara a direccion 0
	if (MACHINE_IS_SPECTRUM_P2A_P3) {
		if ( (puerto_32765 & 32) == 0 ) {
			//paginacion habilitada

			if ( (puerto_8189 & 1) ) {
				//paginacion de ram en rom
				//printf ("paginacion de ram en rom\n");
				inicio_puntero_membuffer=0;
			}
		}
	}

	if (MACHINE_IS_QL) {
		//inicio_puntero_membuffer=0x18000;
		//la ram propiamente empieza en 20000H
		inicio_puntero_membuffer=0x20000;
		final_puntero_membuffer=QL_MEM_LIMIT+1;
	}

	//Si es de opcode o read, puede ser desde cualquier sitio desde la rom
	if (menu_visualmem_donde>0) {
		inicio_puntero_membuffer=0;
	}

	//Valores entre 0 y 255: numero de veces byte modificado
	//Valor 65535 especial
        //int si_modificado;


             //Calcular cuantos bytes modificados representa un pixel, teniendo en cuenta maximo buffer
	int max_valores=(final_puntero_membuffer-inicio_puntero_membuffer)/tamanyo_total;

	//printf ("max_valores: %d\n",max_valores);
	//le damos uno mas para poder llenar la ventana
	//printf ("inicio: %06XH final: %06XH\n",inicio_puntero_membuffer,final_puntero_membuffer);
	max_valores++;

	for (y=yorigen;y<yorigen+alto;y++) {
        for (x=xorigen;x<xorigen+ancho;x++) {

                //Obtenemos conjunto de bytes modificados

                int valores=max_valores;

		int acumulado=0;

		int acumulado_written,acumulado_read,acumulado_opcode;
		acumulado_written=acumulado_read=acumulado_opcode=0; //Estos usados al visualizar los 3 a la vez

		//si_modificado=0;
                for (;valores>0;valores--,inicio_puntero_membuffer++) {
                        if (inicio_puntero_membuffer>=final_puntero_membuffer) {
				//printf ("llegado a final con x: %d y: %d ",x,y);
				//Fuera de memoria direccionable. Zona gris. Decrementamos valor
				//Como se lee a trozos de "max_valores" tamanyo, cuando este trozo empieza ya fuera de memoria
				//acumulado acabara siendo <0 y saldra gris. Si es a medias, si acaba restando mucho, saldra gris tambien
				//(eso solo pasara en el ultimo pixel de la zona direccionable)
				acumulado--;
                        }
			else {
				//Es en memoria direccionable. Sumar valor de visualmem y luego haremos valor medio
				//0: written, 1: read, 2: opcode
				if (menu_visualmem_donde==0) {
					acumulado +=visualmem_buffer[inicio_puntero_membuffer];
					clear_visualmembuffer(inicio_puntero_membuffer);
				}

				else if (menu_visualmem_donde==1) {
					acumulado +=visualmem_read_buffer[inicio_puntero_membuffer];
					clear_visualmemreadbuffer(inicio_puntero_membuffer);
				}

				else if (menu_visualmem_donde==2) {
					acumulado +=visualmem_opcode_buffer[inicio_puntero_membuffer];
					clear_visualmemopcodebuffer(inicio_puntero_membuffer);
				}

				else if (menu_visualmem_donde==3) {
					acumulado_written +=visualmem_buffer[inicio_puntero_membuffer];
					acumulado_read +=visualmem_read_buffer[inicio_puntero_membuffer];
					acumulado_opcode +=visualmem_opcode_buffer[inicio_puntero_membuffer];
					clear_visualmembuffer(inicio_puntero_membuffer);
					clear_visualmemreadbuffer(inicio_puntero_membuffer);
					clear_visualmemopcodebuffer(inicio_puntero_membuffer);
				}				


			}
                }
		//if (acumulado>0) printf ("final pixel %d %d (divisor: %d)\n",inicio_puntero_membuffer,acumulado,max_valores);

            //dibujamos valor medio
            if (acumulado>0 || acumulado_written>0 || acumulado_read>0 || acumulado_opcode>0) {

			if (si_complete_video_driver() ) {

				//Sacar valor medio
				int color_final=acumulado/max_valores;

				//printf ("color final: %d\n",color_final);

				//Aumentar el brillo del color
				color_final=color_final*visualmem_bright_multiplier;
				if (color_final>255) color_final=255;



				//menu_scr_putpixel(x,y,ESTILO_GUI_TINTA_NORMAL);
				//menu_scr_putpixel(x,y,HEATMAP_INDEX_FIRST_COLOR+color_final);
				if (menu_visualmem_donde==3) {
					//Los 3 a la vez. Combinamos color RGB sacando color de paleta tsconf (15 bits)
					//Paleta es RGB R: 5 bits altos, G: 5 bits medios, B:5 bits bajos


					//Sacar valor medio de los 3 componentes
					int color_final_written=acumulado_written/max_valores;
					color_final_written=color_final_written*visualmem_bright_multiplier;
					if (color_final_written>31) color_final_written=31;

					int color_final_read=acumulado_read/max_valores;
					color_final_read=color_final_read*visualmem_bright_multiplier;
					if (color_final_read>31) color_final_read=31;		

					int color_final_opcode=acumulado_opcode/max_valores;
					color_final_opcode=color_final_opcode*visualmem_bright_multiplier;
					if (color_final_opcode>31) color_final_opcode=31;	

					//Blue sera para los written
					//Green sera para los read
					//Red sera para los opcode
					int color_final_rgb=(color_final_opcode<<10)|(color_final_read<<5)|color_final_written;
					zxvision_putpixel(menu_debug_draw_visualmem_window,x,y,TSCONF_INDEX_FIRST_COLOR+color_final_rgb);

				}
				else zxvision_putpixel(menu_debug_draw_visualmem_window,x,y,HEATMAP_INDEX_FIRST_COLOR+color_final);
			}

			else {
				//putchar_menu_overlay(x,y,'#',ESTILO_GUI_TINTA_NORMAL,ESTILO_GUI_PAPEL_NORMAL);
				zxvision_print_char_simple(menu_debug_draw_visualmem_window,x,y,ESTILO_GUI_TINTA_NORMAL,ESTILO_GUI_PAPEL_NORMAL,0,'#');
			}
		}

		//color ficticio para indicar fuera de memoria y por tanto final de ventana... para saber donde acaba
		else if (acumulado<0) {
			if (si_complete_video_driver() ) {
				//menu_scr_putpixel(x,y,ESTILO_GUI_COLOR_UNUSED_VISUALMEM);
				zxvision_putpixel(menu_debug_draw_visualmem_window,x,y,ESTILO_GUI_COLOR_UNUSED_VISUALMEM);
			}
			else {
				//putchar_menu_overlay(x,y,'-',ESTILO_GUI_COLOR_UNUSED_VISUALMEM,ESTILO_GUI_PAPEL_NORMAL);
				zxvision_print_char_simple(menu_debug_draw_visualmem_window,x,y,ESTILO_GUI_TINTA_NORMAL,ESTILO_GUI_PAPEL_NORMAL,0,'-');
            }

		}

		//Valor 0
		else {
			if (si_complete_video_driver() ) {
				//menu_scr_putpixel(x,y,ESTILO_GUI_PAPEL_NORMAL);
				zxvision_putpixel(menu_debug_draw_visualmem_window,x,y,ESTILO_GUI_PAPEL_NORMAL);
			}
			else {
				//putchar_menu_overlay(x,y,' ',ESTILO_GUI_TINTA_NORMAL,ESTILO_GUI_PAPEL_NORMAL);
				zxvision_print_char_simple(menu_debug_draw_visualmem_window,x,y,ESTILO_GUI_TINTA_NORMAL,ESTILO_GUI_PAPEL_NORMAL,0,' ');
			}
		}

        }
	}

	zxvision_draw_window_contents(menu_debug_draw_visualmem_window); 

}









void menu_debug_new_visualmem_looking(MENU_ITEM_PARAMETERS)
{
	menu_visualmem_donde++;
	if (menu_visualmem_donde==4) menu_visualmem_donde=0;
}


void menu_debug_new_visualmem_bright(MENU_ITEM_PARAMETERS)
{
                        if (visualmem_bright_multiplier>=200) visualmem_bright_multiplier=1;
                        else if (visualmem_bright_multiplier==1) visualmem_bright_multiplier=10;
                        else visualmem_bright_multiplier +=10;
}

/*
void menu_debug_new_visualmem_key_o(MENU_ITEM_PARAMETERS)
{
    /if (visualmem_ancho_variable>23) {
		visualmem_ancho_variable--;

		if (visualmem_ancho_variable<VISUALMEM_MAX_ANCHO-1) visualmem_x_variable=VISUALMEM_DEFAULT_X;
	}
}


void menu_debug_new_visualmem_key_p(MENU_ITEM_PARAMETERS)
{
    if (visualmem_ancho_variable<VISUALMEM_MAX_ANCHO) {
		visualmem_ancho_variable++;

		//Mover a la izquierda si maximo
		if (visualmem_ancho_variable>=VISUALMEM_MAX_ANCHO-1) visualmem_x_variable=VISUALMEM_MIN_X;
	}
}

void menu_debug_new_visualmem_key_q(MENU_ITEM_PARAMETERS)
{
    if (visualmem_alto_variable>7) {
		visualmem_alto_variable--;

		if (visualmem_alto_variable<VISUALMEM_MAX_ALTO-1) visualmem_y_variable=VISUALMEM_DEFAULT_Y;
	}
}

void menu_debug_new_visualmem_key_a(MENU_ITEM_PARAMETERS)
{
    if (visualmem_alto_variable<VISUALMEM_MAX_ALTO) {
		visualmem_alto_variable++;

		//Mover a la arriba si maximo
		if (visualmem_alto_variable>=VISUALMEM_MAX_ALTO-1) visualmem_y_variable=VISUALMEM_MIN_Y;		
	}
}
*/
void menu_debug_new_visualmem(MENU_ITEM_PARAMETERS)
{


 	menu_espera_no_tecla();
	menu_reset_counters_tecla_repeticion();		

	zxvision_window ventana;

	zxvision_new_window(&ventana,visualmem_x_variable,visualmem_y_variable,VISUALMEM_DEFAULT_WINDOW_ANCHO,VISUALMEM_DEFAULT_WINDOW_ALTO,
							VISUALMEM_DEFAULT_WINDOW_ANCHO-1,VISUALMEM_DEFAULT_WINDOW_ALTO-2,"Visual memory");
	zxvision_draw_window(&ventana);				


        //Cambiamos funcion overlay de texto de menu
        //Se establece a la de funcion de visualmem + texto
        set_menu_overlay_function(menu_debug_draw_visualmem);


	menu_debug_draw_visualmem_window=&ventana; //Decimos que el overlay lo hace sobre la ventana que tenemos aqui		


	menu_dibuja_menu_permite_repeticiones_hotk=1;


	menu_item *array_menu_debug_new_visualmem;
        menu_item item_seleccionado;
        int retorno_menu;
        do {


	


	//Forzar a mostrar atajos
	z80_bit antes_menu_writing_inverse_color;
	antes_menu_writing_inverse_color.v=menu_writing_inverse_color.v;
	menu_writing_inverse_color.v=1;


	char texto_linea[33];
	//sprintf (texto_linea,"Size: ~~O~~P~~Q~~A ~~Bright: %d",visualmem_bright_multiplier);
	sprintf (texto_linea,"~~Bright: %d",visualmem_bright_multiplier);
	//menu_escribe_linea_opcion(0,-1,1,texto_linea);
	zxvision_print_string_defaults_fillspc(&ventana,1,0,texto_linea);



	if (menu_visualmem_donde == 0) sprintf (texto_linea,"~~Looking: Written Mem");
	else if (menu_visualmem_donde == 1) sprintf (texto_linea,"~~Looking: Read Mem");
	else if (menu_visualmem_donde == 2) sprintf (texto_linea,"~~Looking: Opcode");
	else sprintf (texto_linea,"~~Looking: All");


	//sprintf (texto_linea,"~~Looking: %s",(menu_visualmem_donde == 0 ? "Written Mem" : "Opcode") );
	//menu_escribe_linea_opcion(1,-1,1,texto_linea);
	zxvision_print_string_defaults_fillspc(&ventana,1,1,texto_linea);


	//Restaurar comportamiento atajos
	menu_writing_inverse_color.v=antes_menu_writing_inverse_color.v;



//        char texto_linea[33];
//        sprintf (texto_linea,"Size: ~~O~~P~~Q~~A ~~Bright: %d",visualmem_bright_multiplier);
//        menu_escribe_linea_opcion(VISUALMEM_Y,-1,1,texto_linea);



						menu_add_item_menu_inicial_format(&array_menu_debug_new_visualmem,MENU_OPCION_NORMAL,menu_debug_new_visualmem_bright,NULL,"~~Bright: %d",visualmem_bright_multiplier);
                        //menu_add_item_menu_format(array_menu_debug_new_visualmem,MENU_OPCION_NORMAL,menu_debug_new_visualmem_bright,NULL,"~~Bright: %d",visualmem_bright_multiplier);
                        menu_add_item_menu_shortcut(array_menu_debug_new_visualmem,'b');
                        //menu_add_item_menu_tooltip(array_menu_debug_new_visualmem,"Change bright value");
                        menu_add_item_menu_ayuda(array_menu_debug_new_visualmem,"Change bright value");
			menu_add_item_menu_tabulado(array_menu_debug_new_visualmem,1,0);


			char texto_looking[32];
	        	if (menu_visualmem_donde == 0) sprintf (texto_looking,"Written Mem");
        		else if (menu_visualmem_donde == 1) sprintf (texto_looking,"Read Mem");
		        else if (menu_visualmem_donde == 2) sprintf (texto_looking,"Opcode");
				else sprintf (texto_looking,"All");

                        menu_add_item_menu_format(array_menu_debug_new_visualmem,MENU_OPCION_NORMAL,menu_debug_new_visualmem_looking,NULL,"~~Looking: %s",texto_looking);
                        menu_add_item_menu_shortcut(array_menu_debug_new_visualmem,'l');
                        //menu_add_item_menu_tooltip(array_menu_debug_new_visualmem,"Which visualmem to look at");
                        menu_add_item_menu_ayuda(array_menu_debug_new_visualmem,"Which visualmem to look at. If you select all, the final color will be a RGB color result of:\n"
									"Blue component por Written Mem\nGreen component for Read mem\nRed component for Opcode.\n"
									"Yellow for example is red+green, so opcode fetch+read memory. As an opcode fetch implies a read access,"
									" you won't ever see a red pixel (only opcode fetch) but all opcode fetch will always be yellow.\n"
									"Cyan is green+blue, so read+write\n"
									
									);
                        menu_add_item_menu_tabulado(array_menu_debug_new_visualmem,1,1);



		//Nombre de ventana solo aparece en el caso de stdout
                retorno_menu=menu_dibuja_menu(&debug_new_visualmem_opcion_seleccionada,&item_seleccionado,array_menu_debug_new_visualmem,"Visual memory" );


	//En caso de menus tabulados, es responsabilidad de este de borrar la ventana
	cls_menu_overlay();
                if ((item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu>=0) {
                        //llamamos por valor de funcion
                        if (item_seleccionado.menu_funcion!=NULL) {
                                //printf ("actuamos por funcion\n");
                                item_seleccionado.menu_funcion(item_seleccionado.valor_opcion);
				//En caso de menus tabulados, es responsabilidad de este de borrar la ventana
                                
                        }
                }

        } while ( (item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu!=MENU_RETORNO_ESC && !salir_todos_menus);



	menu_dibuja_menu_permite_repeticiones_hotk=0;



       //restauramos modo normal de texto de menu
       set_menu_overlay_function(normal_overlay_texto_menu);


    cls_menu_overlay();
	//En caso de menus tabulados, es responsabilidad de este de liberar ventana
	zxvision_destroy_window(&ventana);		


}





#endif




void menu_ay_player_load(MENU_ITEM_PARAMETERS)
{
	char *filtros[2];

	filtros[0]="ay";

	filtros[1]=0;

	//guardamos directorio actual
	char directorio_actual[PATH_MAX];
	getcwd(directorio_actual,PATH_MAX);

	//Obtenemos directorio de ultimo archivo
	//si no hay directorio, vamos a rutas predefinidas
	if (last_ay_file[0]==0) menu_chdir_sharedfiles();

	else {
					char directorio[PATH_MAX];
					util_get_dir(last_ay_file,directorio);
					//printf ("strlen directorio: %d directorio: %s\n",strlen(directorio),directorio);

					//cambiamos a ese directorio, siempre que no sea nulo
					if (directorio[0]!=0) {
									debug_printf (VERBOSE_INFO,"Changing to last directory: %s",directorio);
									menu_filesel_chdir(directorio);
					}
	}


	int ret;

	ret=menu_filesel("Select AY File",filtros,last_ay_file);
	//volvemos a directorio inicial
	menu_filesel_chdir(directorio_actual);


	if (ret==1) {


		ay_player_load_and_play(last_ay_file);

	}
}

/*
void menu_ay_player_exit_tracker(MENU_ITEM_PARAMETERS)
{
	ay_player_stop_player();
}
*/

//Retorna indice a string teniendo en cuenta maximo en pantalla e incrementando en 1
int menu_ay_player_get_continuous_string(int indice_actual,int max_length,char *string,int *retardo)
{

	if ( (*retardo)<10 ) {
		(*retardo)++;
		return 0;
	}

	int longitud=strlen(&string[indice_actual]);
	if (longitud<=max_length) {
		indice_actual=0;
		*retardo=0;
	}
	else {
		indice_actual++;
	}

	return indice_actual;
}





int menu_audio_new_ayplayer_si_mostrar(void)
{
	int mostrar_player;


	mostrar_player=1;
	if (audio_ay_player_mem==NULL) mostrar_player=0;
	if (ay_player_playing.v==0) mostrar_player=0;

	return mostrar_player;
}


//Usado dentro del overlay de ayplayer
int menu_ayplayer_valor_contador_segundo_anterior;

int ayplayer_new_contador_string_author=0;
int ayplayer_new_contador_string_track_name=0;
int ayplayer_new_contador_string_misc=0;
int ayplayer_new_retardo_song_name=0;
int ayplayer_new_retardo_author=0;
int ayplayer_new_retardo_misc=0;


//Para hacer las barras de volumen con "caracter" que decae
int ayplayer_previo_valor_escalado=0;

int ayplayer_previo_valor_volume_A=0;
int ayplayer_previo_valor_volume_B=0;
int ayplayer_previo_valor_volume_C=0;

zxvision_window *menu_audio_new_ayplayer_overlay_window;	

void menu_audio_new_ayplayer_overlay(void)
{

    normal_overlay_texto_menu();

                        int linea;


    linea=7;
	int valor_escalado; 

	int vol_A,vol_B,vol_C;

	menu_speech_tecla_pulsada=1; //Si no, envia continuamente todo ese texto a speech


    if (menu_audio_new_ayplayer_si_mostrar()) {
    	//Los volumenes mostrarlos siempre a cada refresco
	char volumen[32];
	char textovolumen[35]; //32+3 de posible color rojo del maximo

	menu_speech_tecla_pulsada=1; //Si no, envia continuamente todo ese texto a speech

	vol_A=ay_3_8912_registros[0][8] & 15;
	vol_B=ay_3_8912_registros[0][9] & 15;
	vol_C=ay_3_8912_registros[0][10] & 15;

	ayplayer_previo_valor_volume_A=menu_decae_ajusta_valor_volumen(ayplayer_previo_valor_volume_A,vol_A);
	ayplayer_previo_valor_volume_B=menu_decae_ajusta_valor_volumen(ayplayer_previo_valor_volume_B,vol_B);
	ayplayer_previo_valor_volume_C=menu_decae_ajusta_valor_volumen(ayplayer_previo_valor_volume_C,vol_C);





	menu_string_volumen(volumen,ay_3_8912_registros[0][8],ayplayer_previo_valor_volume_A);
			sprintf (textovolumen,"Volume A: %s",volumen);
			//menu_escribe_linea_opcion(linea++,-1,1,textovolumen);
			zxvision_print_string_defaults_fillspc(menu_audio_new_ayplayer_overlay_window,1,linea++,textovolumen);

			menu_string_volumen(volumen,ay_3_8912_registros[0][9],ayplayer_previo_valor_volume_B);
			sprintf (textovolumen,"Volume B: %s",volumen);
			//menu_escribe_linea_opcion(linea++,-1,1,textovolumen);
			zxvision_print_string_defaults_fillspc(menu_audio_new_ayplayer_overlay_window,1,linea++,textovolumen);

			menu_string_volumen(volumen,ay_3_8912_registros[0][10],ayplayer_previo_valor_volume_C);
			sprintf (textovolumen,"Volume C: %s",volumen);
			//menu_escribe_linea_opcion(linea++,-1,1,textovolumen);
			zxvision_print_string_defaults_fillspc(menu_audio_new_ayplayer_overlay_window,1,linea++,textovolumen);



	//Obtenemos antes valor medio total y tambien maximo y minimo
	//Esto solo es necesario para dibujar onda llena



        audiobuffer_stats audiostats;
        audio_get_audiobuffer_stats(&audiostats);

	//int volumen_buffer=audiostats.volumen;

	//Ahora tenemos valor entre 0 y 128. Pasar a entre 0 y 15 
	//int valor_escalado=(mayor*16)/128;

	valor_escalado=audiostats.volumen_escalado;

	/*

	int valor_escalado=(volumen_buffer*16)/128;

	//Vigilar que no pase de 15
	if (valor_escalado>15) valor_escalado=15;
	*/

	//printf ("audiomin: %d audiomax: %d maximo: %d valor_escalado: %d\n",audiomin,audiomax,mayor,valor_escalado);

	//Y mostramos indicador volumen
	/*
	Nota: realmente este calculo de volumen no es del todo cierto, estoy viendo el valor maximo de la onda, aunque se puede generar
	sonido muy bajo, por ejemplo, oscilando valores entre 100 y 120 (considerando signed 8 bits), es mas, hay juegos que, al usar beeper,
	"mueven" esa onda hacia arriba, y aunque el indicador de volumen diga que esta muy alto, realmente se oye a volumen normal
	Pero bueno, la mayoria de las veces si que coincide bien el valor de volumen
	*/
	

	ayplayer_previo_valor_escalado=menu_decae_ajusta_valor_volumen(ayplayer_previo_valor_escalado,valor_escalado);
	//if (ayplayer_previo_valor_escalado<valor_escalado) ayplayer_previo_valor_escalado=valor_escalado;

			menu_string_volumen(volumen,valor_escalado,ayplayer_previo_valor_escalado);




								//"Volume C: %s"
			sprintf (textovolumen,"Output:   %s",volumen);
			//menu_escribe_linea_opcion(linea++,-1,1,textovolumen);
			zxvision_print_string_defaults_fillspc(menu_audio_new_ayplayer_overlay_window,1,linea++,textovolumen);

	}

	else {
		//Borrar lineas
		int i;
		linea=0;
		for (i=0;i<11;i++) zxvision_print_string_defaults_fillspc(menu_audio_new_ayplayer_overlay_window,1,linea++,"");
	}

    //esto hara ejecutar esto 2 veces por segundo
    if ( ((contador_segundo%500) == 0 && menu_ayplayer_valor_contador_segundo_anterior!=contador_segundo) || menu_multitarea==0) {

        menu_ayplayer_valor_contador_segundo_anterior=contador_segundo;
        //printf ("Refrescando. contador_segundo=%d\n",contador_segundo);
       
    


char textoplayer[40];

       


int mostrar_player;



	mostrar_player=menu_audio_new_ayplayer_si_mostrar();


         


		if (mostrar_player) {

			linea=0;

	//Indicadores de volumen que decaen
	ayplayer_previo_valor_escalado=menu_decae_dec_valor_volumen(ayplayer_previo_valor_escalado,valor_escalado);


	ayplayer_previo_valor_volume_A=menu_decae_dec_valor_volumen(ayplayer_previo_valor_volume_A,vol_A);
	ayplayer_previo_valor_volume_B=menu_decae_dec_valor_volumen(ayplayer_previo_valor_volume_B,vol_B);
	ayplayer_previo_valor_volume_C=menu_decae_dec_valor_volumen(ayplayer_previo_valor_volume_C,vol_C);




			//printf ("Dibujando player\n");

			z80_byte minutos,segundos,minutos_total,segundos_total;
			minutos=ay_song_length_counter/60/50;
			segundos=(ay_song_length_counter/50)%60;
			minutos_total=ay_song_length/60/50;
			segundos_total=(ay_song_length/50)%60;

//printf ("segundo. contador segundo: %d\n",contador_segundo);

			sprintf (textoplayer,"Track: %03d/%03d  (%02d:%02d/%02d:%02d)",ay_player_pista_actual,ay_player_total_songs(),minutos,segundos,minutos_total,segundos_total);
			//menu_escribe_linea_opcion(linea++,-1,1,textoplayer);
			zxvision_print_string_defaults_fillspc(menu_audio_new_ayplayer_overlay_window,1,linea++,textoplayer);


			strncpy(textoplayer,&ay_player_file_song_name[ayplayer_new_contador_string_track_name],28);
			textoplayer[28]=0;
			//menu_escribe_linea_opcion(linea++,-1,1,textoplayer);
			zxvision_print_string_defaults_fillspc(menu_audio_new_ayplayer_overlay_window,1,linea++,textoplayer);
			ayplayer_new_contador_string_track_name=menu_ay_player_get_continuous_string(ayplayer_new_contador_string_track_name,28,ay_player_file_song_name,&ayplayer_new_retardo_song_name);

			//menu_escribe_linea_opcion(linea++,-1,1,"Author");
			zxvision_print_string_defaults_fillspc(menu_audio_new_ayplayer_overlay_window,1,linea++,"Author");
			strncpy(textoplayer,&ay_player_file_author[ayplayer_new_contador_string_author],28);
			textoplayer[28]=0;
			//menu_escribe_linea_opcion(linea++,-1,1,textoplayer);
			zxvision_print_string_defaults_fillspc(menu_audio_new_ayplayer_overlay_window,1,linea++,textoplayer);
			ayplayer_new_contador_string_author=menu_ay_player_get_continuous_string(ayplayer_new_contador_string_author,28,ay_player_file_author,&ayplayer_new_retardo_author);

			//menu_escribe_linea_opcion(linea++,-1,1,"Misc");
			zxvision_print_string_defaults_fillspc(menu_audio_new_ayplayer_overlay_window,1,linea++,"Misc");
			strncpy(textoplayer,&ay_player_file_misc[ayplayer_new_contador_string_misc],28);
			textoplayer[28]=0;
			//menu_escribe_linea_opcion(linea++,-1,1,textoplayer);
			zxvision_print_string_defaults_fillspc(menu_audio_new_ayplayer_overlay_window,1,linea++,textoplayer);
			ayplayer_new_contador_string_misc=menu_ay_player_get_continuous_string(ayplayer_new_contador_string_misc,28,ay_player_file_misc,&ayplayer_new_retardo_misc);




		}
	}

	zxvision_draw_window_contents(menu_audio_new_ayplayer_overlay_window);
}



void menu_audio_new_ayplayer_load(MENU_ITEM_PARAMETERS)
{

	//restauramos modo normal de texto de menu
        set_menu_overlay_function(normal_overlay_texto_menu);

	
	menu_ay_player_load(0);

	//Restauramos funcion de overlay
	set_menu_overlay_function(menu_audio_new_ayplayer_overlay);

}

void menu_audio_new_ayplayer_prev(MENU_ITEM_PARAMETERS)
{
	ay_player_previous_track();

}

void menu_audio_new_ayplayer_next(MENU_ITEM_PARAMETERS)
{
	ay_player_next_track();

}

void menu_audio_new_ayplayer_stop(MENU_ITEM_PARAMETERS)
{
	ay_player_stop_player();

}

void menu_audio_new_ayplayer_repeat(MENU_ITEM_PARAMETERS)
{
	ay_player_repeat_file.v ^=1;

}


void menu_audio_new_ayplayer_exitend(MENU_ITEM_PARAMETERS)
{
	ay_player_exit_emulator_when_finish.v ^=1;
}

void menu_audio_new_ayplayer_cpcmode(MENU_ITEM_PARAMETERS)
{
	ay_player_cpc_mode.v ^=1;
	audio_ay_player_play_song(ay_player_pista_actual);
}

												

void menu_audio_new_ayplayer_inftracks(MENU_ITEM_PARAMETERS)
{
	//restauramos modo normal de texto de menu
        set_menu_overlay_function(normal_overlay_texto_menu);

	
	char string_length[5];
	sprintf(string_length,"%d",ay_player_limit_infinite_tracks/50);

        menu_ventana_scanf("Length (0-1310)",string_length,5);
	int l=parse_string_to_number(string_length);

	if (l<0 || l>1310) {
		menu_error_message("Invalid length value");
	}

	else ay_player_limit_infinite_tracks=l*50;

	

	//Restauramos funcion de overlay
	set_menu_overlay_function(menu_audio_new_ayplayer_overlay);
}

void menu_audio_new_ayplayer_len_anytracks(MENU_ITEM_PARAMETERS)
{

	//restauramos modo normal de texto de menu
        set_menu_overlay_function(normal_overlay_texto_menu);

	char string_length[5];
	sprintf(string_length,"%d",ay_player_limit_any_track/50);

	menu_ventana_scanf("Length (0-1310)",string_length,5);
	int l=parse_string_to_number(string_length);

	if (l<0 || l>1310) {
		menu_error_message("Invalid length value");
	}

	else ay_player_limit_any_track=l*50;


	//Restauramos funcion de overlay
	set_menu_overlay_function(menu_audio_new_ayplayer_overlay);
}



void menu_audio_new_ayplayer(MENU_ITEM_PARAMETERS)
{


        //menu_espera_no_tecla();




				if (!menu_multitarea) {
					menu_warn_message("This menu item needs multitask enabled");
					return;
				}

 	menu_espera_no_tecla();
	menu_reset_counters_tecla_repeticion();		

	zxvision_window ventana;

	zxvision_new_window(&ventana,0,1,32,20,
							32-1,20-2,"AY Player");
	zxvision_draw_window(&ventana);			


        //Cambiamos funcion overlay de texto de menu
        //Se establece a la de funcion de audio waveform
	set_menu_overlay_function(menu_audio_new_ayplayer_overlay);


	menu_audio_new_ayplayer_overlay_window=&ventana; //Decimos que el overlay lo hace sobre la ventana que tenemos aqui		



	menu_item *array_menu_audio_new_ayplayer;
        menu_item item_seleccionado;
        int retorno_menu;
        do {

        			ayplayer_new_contador_string_author=0;
				ayplayer_new_contador_string_track_name=0;
				ayplayer_new_contador_string_misc=0;

 				ayplayer_new_retardo_song_name=0;
				ayplayer_new_retardo_author=0;
				ayplayer_new_retardo_misc=0;

        	char textoplayer[40];
            //Hay que redibujar la ventana desde este bucle
            //menu_audio_new_ayplayer_dibuja_ventana();



            menu_add_item_menu_inicial_format(&array_menu_audio_new_ayplayer,MENU_OPCION_NORMAL,menu_audio_new_ayplayer_load,NULL,"~~Load");
            menu_add_item_menu_shortcut(array_menu_audio_new_ayplayer,'l');
            menu_add_item_menu_ayuda(array_menu_audio_new_ayplayer,"Load AY file");

						
			

			int lin=12;

			menu_add_item_menu_tabulado(array_menu_audio_new_ayplayer,1,lin+5);

				//Vamos a borrar con espacios para que no quede rastro de opciones anteriores, como Yes/No 
				//Si no, pasaria que mostraria "Nos" como parte de la s final de Yes
				int i;
				for (i=12;i<=16;i++) {
					zxvision_fill_width_spaces(&ventana,i);
				}			

			if (menu_audio_new_ayplayer_si_mostrar() ) {


				menu_add_item_menu_format(array_menu_audio_new_ayplayer,MENU_OPCION_NORMAL,menu_audio_new_ayplayer_prev,NULL,"~~Prev");
				menu_add_item_menu_shortcut(array_menu_audio_new_ayplayer,'p');
				menu_add_item_menu_ayuda(array_menu_audio_new_ayplayer,"Previous song");
				menu_add_item_menu_tabulado(array_menu_audio_new_ayplayer,1,lin);

				menu_add_item_menu_format(array_menu_audio_new_ayplayer,MENU_OPCION_NORMAL,menu_audio_new_ayplayer_stop,NULL,"~~Stop");
				menu_add_item_menu_shortcut(array_menu_audio_new_ayplayer,'s');
				menu_add_item_menu_ayuda(array_menu_audio_new_ayplayer,"Stop song");
				menu_add_item_menu_tabulado(array_menu_audio_new_ayplayer,6,lin);	

				menu_add_item_menu_format(array_menu_audio_new_ayplayer,MENU_OPCION_NORMAL,menu_audio_new_ayplayer_next,NULL,"~~Next");
				menu_add_item_menu_shortcut(array_menu_audio_new_ayplayer,'n');
				menu_add_item_menu_ayuda(array_menu_audio_new_ayplayer,"Next song");
				menu_add_item_menu_tabulado(array_menu_audio_new_ayplayer,11,lin);

				menu_add_item_menu_format(array_menu_audio_new_ayplayer,MENU_OPCION_NORMAL,menu_audio_new_ayplayer_repeat,NULL,"[%c] Repeat",
					(ay_player_repeat_file.v ? 'X' : ' '));

				menu_add_item_menu_shortcut(array_menu_audio_new_ayplayer,'r');
				menu_add_item_menu_ayuda(array_menu_audio_new_ayplayer,"Repeat from the beginning when finished all songs");
				menu_add_item_menu_tabulado(array_menu_audio_new_ayplayer,1,lin+1);	

				
				menu_add_item_menu_format(array_menu_audio_new_ayplayer,MENU_OPCION_NORMAL,menu_audio_new_ayplayer_exitend,NULL,"[%c] Exit end",
					(ay_player_exit_emulator_when_finish.v ? 'X' : ' ') );
				menu_add_item_menu_shortcut(array_menu_audio_new_ayplayer,'e');
				menu_add_item_menu_ayuda(array_menu_audio_new_ayplayer,"Exit emulator when finished all songs");
				menu_add_item_menu_tabulado(array_menu_audio_new_ayplayer,13,lin+1);	


				if (ay_player_limit_infinite_tracks==0) sprintf(textoplayer,"Length ~~infinite tracks: inf");
				else sprintf(textoplayer,"Length ~~infinite tracks: %d s",ay_player_limit_infinite_tracks/50);
				menu_add_item_menu_format(array_menu_audio_new_ayplayer,MENU_OPCION_NORMAL,menu_audio_new_ayplayer_inftracks,NULL,textoplayer);
				menu_add_item_menu_shortcut(array_menu_audio_new_ayplayer,'i');
				menu_add_item_menu_ayuda(array_menu_audio_new_ayplayer,"Time limit for songs which doesn't have time limit");
				menu_add_item_menu_tabulado(array_menu_audio_new_ayplayer,1,lin+2);			


				if (ay_player_limit_any_track==0) sprintf(textoplayer,"Length ~~any track: No limit");
				else sprintf(textoplayer,"Length ~~any track: %d s",ay_player_limit_any_track/50);
				menu_add_item_menu_format(array_menu_audio_new_ayplayer,MENU_OPCION_NORMAL,menu_audio_new_ayplayer_len_anytracks,NULL,textoplayer);
				menu_add_item_menu_shortcut(array_menu_audio_new_ayplayer,'a');
				menu_add_item_menu_ayuda(array_menu_audio_new_ayplayer,"Time limit for all songs");
				menu_add_item_menu_tabulado(array_menu_audio_new_ayplayer,1,lin+3);

				menu_add_item_menu_format(array_menu_audio_new_ayplayer,MENU_OPCION_NORMAL,menu_audio_new_ayplayer_cpcmode,NULL,"[%c] CPC mode",
					(ay_player_cpc_mode.v ? 'X' : ' '));
				menu_add_item_menu_shortcut(array_menu_audio_new_ayplayer,'c');
				menu_add_item_menu_ayuda(array_menu_audio_new_ayplayer,"Switch to AY CPC mode");
				menu_add_item_menu_tabulado(array_menu_audio_new_ayplayer,1,lin+4);		


			}			
/*


			sprintf(textoplayer,"~~CPC mode: %s",(ay_player_cpc_mode.v ? 'X' : ' '));
			menu_escribe_linea_opcion(linea++,-1,1,textoplayer);
*/



		//Nombre de ventana solo aparece en el caso de stdout
                retorno_menu=menu_dibuja_menu(&audio_new_ayplayer_opcion_seleccionada,&item_seleccionado,array_menu_audio_new_ayplayer,"AY Player" );


	//En caso de menus tabulados, es responsabilidad de este de borrar la ventana
	cls_menu_overlay();
                if ((item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu>=0) {
                        //llamamos por valor de funcion
                        if (item_seleccionado.menu_funcion!=NULL) {
                                //printf ("actuamos por funcion\n");
                                item_seleccionado.menu_funcion(item_seleccionado.valor_opcion);
				//En caso de menus tabulados, es responsabilidad de este de borrar la ventana
                                
                        }
                }

        } while ( (item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu!=MENU_RETORNO_ESC && !salir_todos_menus);



       //restauramos modo normal de texto de menu
       set_menu_overlay_function(normal_overlay_texto_menu);


        cls_menu_overlay();

	//En caso de menus tabulados, es responsabilidad de este de liberar ventana
	zxvision_destroy_window(&ventana);				

}








#define DEBUG_HEXDUMP_WINDOW_X 0
#define DEBUG_HEXDUMP_WINDOW_Y 1
#define DEBUG_HEXDUMP_WINDOW_ANCHO 32
#define DEBUG_HEXDUMP_WINDOW_ALTO 23



void menu_debug_hexdump_with_ascii(char *dumpmemoria,menu_z80_moto_int dir_leida,int bytes_por_linea,z80_byte valor_xor)
{
	//dir_leida=adjust_address_space_cpu(dir_leida);

	menu_debug_set_memory_zone_attr();


	int longitud_direccion=MAX_LENGTH_ADDRESS_MEMORY_ZONE;

	menu_debug_print_address_memory_zone(dumpmemoria,dir_leida);

	

	//cambiamos el 0 final por un espacio
	dumpmemoria[longitud_direccion]=' ';

	menu_debug_registers_dump_hex(&dumpmemoria[longitud_direccion+1],dir_leida,bytes_por_linea);

	//01234567890123456789012345678901
	//000FFF ABCDABCDABCDABCD 12345678

	//metemos espacio
	int offset=longitud_direccion+1+bytes_por_linea*2;

	dumpmemoria[offset]=' ';
	//dumpmemoria[offset]='X';

	//Tener en cuenta el valor xor

	menu_debug_registers_dump_ascii(&dumpmemoria[offset+1],dir_leida,bytes_por_linea,menu_debug_hexdump_with_ascii_modo_ascii,valor_xor);

	//printf ("%s\n",dumpmemoria);
}


menu_z80_moto_int menu_debug_hexdump_direccion=0;

int menu_hexdump_edit_position_x=0; //Posicion del cursor relativa al inicio del volcado hexa
int menu_hexdump_edit_position_y=0; //Posicion del cursor relativa al inicio del volcado hexa
const int menu_hexdump_lineas_total=13;

int menu_hexdump_edit_mode=0;
const int menu_hexdump_bytes_por_linea=8;

void menu_debug_hexdump_cursor_abajo(void);
void menu_debug_hexdump_cursor_arriba(void);



void menu_debug_hexdump_print_editcursor(zxvision_window *ventana,int x,int y,char caracter)
{
	//z80_byte papel=ESTILO_GUI_PAPEL_NORMAL;
    //z80_byte tinta=ESTILO_GUI_TINTA_NORMAL;

	//Inverso
	z80_byte papel=ESTILO_GUI_PAPEL_SELECCIONADO;
    z80_byte tinta=ESTILO_GUI_TINTA_SELECCIONADO;	

	//Si multitarea esta off, no se vera el parpadeo. Entonces cambiar el caracter por cursor '_'
	if (!menu_multitarea) caracter='_';

	//putchar_menu_overlay_parpadeo(x,y,caracter,tinta,papel,1);
	zxvision_print_char_simple(ventana,x,y,tinta,papel,1,caracter);

}

void menu_debug_hexdump_print_editcursor_nibble(zxvision_window *ventana,int x,int y,char caracter)
{
	//z80_byte papel=ESTILO_GUI_PAPEL_NORMAL;
    //z80_byte tinta=ESTILO_GUI_TINTA_NORMAL;

	//Inverso
	z80_byte papel=ESTILO_GUI_PAPEL_SELECCIONADO;
    z80_byte tinta=ESTILO_GUI_TINTA_SELECCIONADO;	

	//putchar_menu_overlay_parpadeo(x,y,caracter,tinta,papel,0);
	zxvision_print_char_simple(ventana,x,y,tinta,papel,0,caracter);

}

void menu_debug_hexdump_edit_cursor_izquierda(void)
{
	if (menu_hexdump_edit_position_x>0) {
		menu_hexdump_edit_position_x--;

		//Si en medio del espacio entre hexa y ascii
		if (menu_hexdump_edit_position_x==menu_hexdump_bytes_por_linea*2) menu_hexdump_edit_position_x--;
	}

	else {
		//Aparecer por la derecha
		menu_debug_hexdump_cursor_arriba();
		menu_hexdump_edit_position_x=menu_hexdump_bytes_por_linea*3;
	}

}

//escribiendo_memoria cursor indica que si estamos a la derecha de zona de edicion escribiendo,
//tiene que saltar a la zona izquierda de la zona ascii o hexa, al llegar a la derecha de dicha zona
void menu_debug_hexdump_edit_cursor_derecha(int escribiendo_memoria)
{

	//Hexdump. bytes_por_linea*2 espacio bytes_por_linea

	int ancho_linea=menu_hexdump_bytes_por_linea*3+1;

	if (menu_hexdump_edit_position_x<ancho_linea-1) {
		menu_hexdump_edit_position_x++;

		if (menu_hexdump_edit_position_x==menu_hexdump_bytes_por_linea*2) { //Fin zona derecha hexa
			if (escribiendo_memoria) {
				//Ponernos al inicio zona hexa de nuevo saltando siguiente linea
				menu_hexdump_edit_position_x=0;
				menu_debug_hexdump_cursor_abajo();
			}
			else {
				//Saltar a zona ascii
				menu_hexdump_edit_position_x++;
			}
		}
	}

	else {
		//Fin zona derecha ascii. 
		menu_debug_hexdump_cursor_abajo();

		if (escribiendo_memoria) {
			//Ponernos en el principio zona ascii
			menu_hexdump_edit_position_x=menu_hexdump_bytes_por_linea*2+1;
		}
		else {
			menu_hexdump_edit_position_x=0;
		}
	}

}

void menu_debug_hexdump_cursor_arriba(void)
{
	int alterar_ptr=0;
						//arriba
						if (menu_hexdump_edit_mode) {
							if (menu_hexdump_edit_position_y>0) menu_hexdump_edit_position_y--;
							else alterar_ptr=1;
						}

						else {
							alterar_ptr=1;
						}

						if (alterar_ptr) {
							menu_debug_hexdump_direccion -=menu_hexdump_bytes_por_linea;
							menu_debug_hexdump_direccion=menu_debug_hexdump_adjusta_en_negativo(menu_debug_hexdump_direccion,menu_hexdump_bytes_por_linea);
						}
}

void menu_debug_hexdump_cursor_abajo(void)
{
	int alterar_ptr=0;
						//abajo
						if (menu_hexdump_edit_mode) {
							if (menu_hexdump_edit_position_y<menu_hexdump_lineas_total-1) menu_hexdump_edit_position_y++;
							else alterar_ptr=1;
						}						
						else {
							alterar_ptr=1;
						}

						if (alterar_ptr) {
							menu_debug_hexdump_direccion +=menu_hexdump_bytes_por_linea;
						}
}

void menu_debug_hexdump_copy(void)
{


    char string_address[10];

    sprintf (string_address,"%XH",menu_debug_hexdump_direccion);
    menu_ventana_scanf("Source?",string_address,10);
	menu_z80_moto_int source=parse_string_to_number(string_address);

    sprintf (string_address,"%XH",source);
    menu_ventana_scanf("Destination?",string_address,10);
	menu_z80_moto_int destination=parse_string_to_number(string_address);

    strcpy (string_address,"1");
    menu_ventana_scanf("Length?",string_address,10);
	menu_z80_moto_int longitud=parse_string_to_number(string_address);	

	if (menu_confirm_yesno("Copy bytes")) {
		for (;longitud>0;source++,destination++,longitud--) poke_byte_z80_moto(destination,peek_byte_z80_moto(source) );
	}


}

void menu_debug_hexdump_aviso_edit_filezone(zxvision_window *w)
{
							menu_warn_message("Memory zone is File zone. Changes won't be saved to the file");
							//Volver a dibujar ventana, pues se ha borrado al aparecer el aviso
							//menu_debug_hexdump_ventana();	
	zxvision_draw_window(w);
}

void menu_debug_hexdump_info_subzones(void)
{
	
		int x=1;
		int y=1;
		int ancho=30;
		int alto=22;



        subzone_info *puntero;
        puntero=machine_get_memory_subzone_array(menu_debug_memory_zone,current_machine_type);
        if (puntero==NULL) return;

		zxvision_window ventana;

                zxvision_new_window(&ventana,x,y,ancho,alto,
                                                        64,alto-2,"Memory subzones");

                zxvision_draw_window(&ventana);		

        int i;

		char buffer_linea[64];
        for (i=0;puntero[i].nombre[0]!=0;i++) {

			//printf ("inicio: %d fin: %d texto: %s\n",puntero[i].inicio,puntero[i].fin,puntero[i].nombre);
			sprintf (buffer_linea,"%06X-%06X %s",puntero[i].inicio,puntero[i].fin,puntero[i].nombre);
			zxvision_print_string_defaults_fillspc(&ventana,1,i,buffer_linea);
			
		}

		zxvision_draw_window_contents(&ventana);

		zxvision_wait_until_esc(&ventana);

        cls_menu_overlay();

                zxvision_destroy_window(&ventana);					


}

void menu_debug_hexdump(MENU_ITEM_PARAMETERS)
{
	menu_espera_no_tecla();
	menu_reset_counters_tecla_repeticion();		

	zxvision_window ventana;

	//asignamos mismo ancho visible que ancho total para poder usar la ultima columna de la derecha, donde se suele poner scroll vertical
	zxvision_new_window(&ventana,DEBUG_HEXDUMP_WINDOW_X,DEBUG_HEXDUMP_WINDOW_Y,DEBUG_HEXDUMP_WINDOW_ANCHO,DEBUG_HEXDUMP_WINDOW_ALTO,
							DEBUG_HEXDUMP_WINDOW_ANCHO,DEBUG_HEXDUMP_WINDOW_ALTO-2,"Hexadecimal Editor");


	ventana.can_use_all_width=1; //Para poder usar la ultima columna de la derecha donde normalmente aparece linea scroll

	zxvision_draw_window(&ventana);


    z80_byte tecla;

	int salir=0;

	z80_byte valor_xor=0;



	if (MACHINE_IS_ZX80) menu_debug_hexdump_with_ascii_modo_ascii=1;
	else if (MACHINE_IS_ZX81) menu_debug_hexdump_with_ascii_modo_ascii=2;

	else menu_debug_hexdump_with_ascii_modo_ascii=0;


	int asked_about_writing_rom=0;

                //Guardar estado atajos
                z80_bit antes_menu_writing_inverse_color;
                antes_menu_writing_inverse_color.v=menu_writing_inverse_color.v;	



        do {

			int cursor_en_zona_ascii=0;
			int editando_en_zona_ascii=0;


					//Si maquina no es QL, direccion siempre entre 0 y 65535
					//menu_debug_hexdump_direccion=adjust_address_space_cpu(menu_debug_hexdump_direccion);
					menu_debug_hexdump_direccion=adjust_address_memory_size(menu_debug_hexdump_direccion);


		int linea=0;

		int lineas_hex=0;



		int bytes_por_ventana=menu_hexdump_bytes_por_linea*menu_hexdump_lineas_total;

		char dumpmemoria[33];



		//Antes de escribir, normalizar zona memoria
		menu_debug_set_memory_zone_attr();

				char textoshow[33];

		sprintf (textoshow,"Showing %d bytes per page:",bytes_por_ventana);
        //menu_escribe_linea_opcion(linea++,-1,1,textoshow);
		zxvision_print_string_defaults_fillspc(&ventana,1,linea++,textoshow);

        //menu_escribe_linea_opcion(linea++,-1,1,"");
		zxvision_print_string_defaults_fillspc(&ventana,1,linea++,"");


		//Hacer que texto ventana empiece pegado a la izquierda
		menu_escribe_linea_startx=0;

		//No mostrar caracteres especiales
		menu_disable_special_chars.v=1;

		//Donde esta el otro caracter que acompanya al nibble, en caso de cursor en zona hexa
		int menu_hexdump_edit_position_x_nibble=menu_hexdump_edit_position_x^1;

		
		if (menu_hexdump_edit_position_x>menu_hexdump_bytes_por_linea*2) cursor_en_zona_ascii=1;


		if (menu_hexdump_edit_mode && cursor_en_zona_ascii) editando_en_zona_ascii=1;		

		char nibble_char='X';	
		char nibble_char_cursor='X';	

		for (;lineas_hex<menu_hexdump_lineas_total;lineas_hex++,linea++) {

			menu_z80_moto_int dir_leida=menu_debug_hexdump_direccion+lineas_hex*menu_hexdump_bytes_por_linea;
			menu_debug_hexdump_direccion=adjust_address_memory_size(menu_debug_hexdump_direccion);

			menu_debug_hexdump_with_ascii(dumpmemoria,dir_leida,menu_hexdump_bytes_por_linea,valor_xor);

			zxvision_print_string_defaults_fillspc(&ventana,0,linea,dumpmemoria);

			//Meter el nibble_char si corresponde
			if (lineas_hex==menu_hexdump_edit_position_y) {
				nibble_char_cursor=dumpmemoria[7+menu_hexdump_edit_position_x];
				if (!editando_en_zona_ascii) nibble_char=dumpmemoria[7+menu_hexdump_edit_position_x_nibble];
			}
		}


		menu_escribe_linea_startx=1;

		//Volver a mostrar caracteres especiales
		menu_disable_special_chars.v=0;		



		//Mostrar cursor si en modo edicion
		
		//Al mostrar en cursor: si esta en parte ascii, hacer parpadear el caracter en esa zona, metiendo color de opcion seleccionada
		//Si esta en parte hexa, parpadeamos la parte del nibble que editamos, el otro nibble no parpadea. Ambos tienen color de opcion seleccionada
		//Si multitarea esta a off, no existe el parpadeo, y por tanto, para que se viera en que nibble edita, se mostrara el caracter _, logicamente
		//tapando el caracter de debajo
		//Para ver los caracteres de debajo, los asignamos antes, en el bucle que hace el volcado hexa, y lo guardo en las variables
		//nibble_char_cursor (que dice el caracter de debajo del cursor) y nibble_char (que dice el otro caracter que acompanya al nibble)
	
		
		if (menu_hexdump_edit_mode) {
			//int xfinal=DEBUG_HEXDUMP_WINDOW_X+7+menu_hexdump_edit_position_x;
			//int yfinal=DEBUG_HEXDUMP_WINDOW_Y+3+menu_hexdump_edit_position_y;
			int xfinal=7+menu_hexdump_edit_position_x;
			int yfinal=2+menu_hexdump_edit_position_y;			

			menu_debug_hexdump_print_editcursor(&ventana,xfinal,yfinal,nibble_char_cursor);

			//Indicar nibble entero. En caso de edit hexa
			if (!editando_en_zona_ascii) {
				//xfinal=DEBUG_HEXDUMP_WINDOW_X+7+menu_hexdump_edit_position_x_nibble;
				xfinal=7+menu_hexdump_edit_position_x_nibble;
				menu_debug_hexdump_print_editcursor_nibble(&ventana,xfinal,yfinal,nibble_char);
			}
		}


				//Forzar a mostrar atajos
                menu_writing_inverse_color.v=1;


//printf ("zone size: %x dir: %x\n",menu_debug_memory_zone_size,menu_debug_hexdump_direccion);

        //menu_escribe_linea_opcion(linea++,-1,1,"");
		zxvision_print_string_defaults_fillspc(&ventana,1,linea++,"");

		char buffer_linea[64]; //Por si acaso, entre negritas y demas

		char buffer_char_type[20];

		char string_atajos[3]="~~"; 
		//Si esta en edit mode y en zona de ascii, no hay atajos



		if (editando_en_zona_ascii) string_atajos[0]=0;

				if (menu_debug_hexdump_with_ascii_modo_ascii==0) {
					sprintf (buffer_char_type,"ASCII");
				}

				else if (menu_debug_hexdump_with_ascii_modo_ascii==1) {
                	sprintf (buffer_char_type,"ZX80");
                }

				else sprintf (buffer_char_type,"ZX81");



				sprintf (buffer_linea,"%sMemptr C%sopy",string_atajos,string_atajos);


				//menu_escribe_linea_opcion(linea++,-1,1,buffer_linea);
				zxvision_print_string_defaults_fillspc(&ventana,1,linea++,buffer_linea);

				sprintf (buffer_linea,"%sInvert:%s Edi%st:%s C%shar:%s",
					string_atajos,
					(valor_xor==0 ? "No" : "Yes"), 
					string_atajos,
					(menu_hexdump_edit_mode==0 ? "No" : "Yes" ),
					string_atajos,
					buffer_char_type
					);
				//menu_escribe_linea_opcion(linea++,-1,1,buffer_linea);
				zxvision_print_string_defaults_fillspc(&ventana,1,linea++,buffer_linea);


				char memory_zone_text[64]; //espacio temporal mas grande por si acaso
				if (menu_debug_show_memory_zones==0) {
					sprintf (memory_zone_text,"Mem %szone (mapped memory)",string_atajos);
				}
				else {
					//printf ("Info zona %d\n",menu_debug_memory_zone);
					char buffer_name[MACHINE_MAX_MEMORY_ZONE_NAME_LENGHT+1];
					//int readwrite;
					machine_get_memory_zone_name(menu_debug_memory_zone,buffer_name);
					sprintf (memory_zone_text,"Mem %szone (%d %s)",string_atajos,menu_debug_memory_zone,buffer_name);
					//printf ("size: %X\n",menu_debug_memory_zone_size);
					//printf ("Despues zona %d\n",menu_debug_memory_zone);
				}

				//truncar texto a 32 por si acaso
				memory_zone_text[32]=0;
				//menu_escribe_linea_opcion(linea++,-1,1,memory_zone_text);
				zxvision_print_string_defaults_fillspc(&ventana,1,linea++,memory_zone_text);

				sprintf (textoshow," Size: %d (%d KB)",menu_debug_memory_zone_size,menu_debug_memory_zone_size/1024);
				//menu_escribe_linea_opcion(linea++,-1,1,textoshow);
				zxvision_print_string_defaults_fillspc(&ventana,1,linea++,textoshow);

		
				char subzone_info[33];
				machine_get_memory_subzone_name(menu_debug_memory_zone,current_machine_type, menu_debug_hexdump_direccion, subzone_info);
				if (subzone_info[0]!=0) {
					sprintf(buffer_linea," S~~ubzone info: %s",subzone_info);
					zxvision_print_string_defaults_fillspc(&ventana,1,linea++,buffer_linea);
				}
				else {
					zxvision_print_string_defaults_fillspc(&ventana,1,linea++,"");
				}


//Restaurar comportamiento atajos
menu_writing_inverse_color.v=antes_menu_writing_inverse_color.v;

			zxvision_draw_window_contents(&ventana);
			//NOTA: este menu no acostumbra a refrescar rapido la ventana cuando la redimensionamos con el raton
			//es una razon facil: el volcado de hexa usa relativamente mucha cpu,
			//cada vez que redimensionamos ventana, se llama al bucle continuamente, usando mucha cpu y si esta el autoframeskip,
			//hace saltar frames
			

			tecla=zxvision_common_getkey_refresh();		


				//Aviso: hay que conseguir que las letras de accion no esten entre la a-f, porque asi,
				//podemos usar dichas letras para editar hexa

				switch (tecla) {

					case 11:
						menu_debug_hexdump_cursor_arriba();

					break;

					case 10:
						menu_debug_hexdump_cursor_abajo();

					break;

					case 8:
					case 12:
						//izquierda o delete
						if (menu_hexdump_edit_mode) {
							//if (menu_hexdump_edit_position_x>0) menu_hexdump_edit_position_x--;
							menu_debug_hexdump_edit_cursor_izquierda();
						}
					break;

					case 9:
						//derecha
						if (menu_hexdump_edit_mode) {
							menu_debug_hexdump_edit_cursor_derecha(0);
							//if (menu_hexdump_edit_position_x<(bytes_por_linea*2)-1) menu_hexdump_edit_position_x++;
						}
					break;					

					case 24:
						//PgUp
						menu_debug_hexdump_direccion -=bytes_por_ventana;
						menu_debug_hexdump_direccion=menu_debug_hexdump_adjusta_en_negativo(menu_debug_hexdump_direccion,bytes_por_ventana);
					break;

					case 25:
						//PgDn
						menu_debug_hexdump_direccion +=bytes_por_ventana;
					break;

					case 'm':
						if (!editando_en_zona_ascii)  {
							menu_debug_hexdump_direccion=menu_debug_hexdump_change_pointer(menu_debug_hexdump_direccion);
							//menu_debug_hexdump_ventana();
							zxvision_draw_window(&ventana);
						}
					break;

					case 'o':
						if (!editando_en_zona_ascii)  {
							menu_debug_hexdump_copy();
							//menu_debug_hexdump_ventana();
							zxvision_draw_window(&ventana);
						}
					break;					

					case 'h':
						if (!editando_en_zona_ascii)  {
							menu_debug_hexdump_with_ascii_modo_ascii++;
							if (menu_debug_hexdump_with_ascii_modo_ascii==3) menu_debug_hexdump_with_ascii_modo_ascii=0;
						}
					break;

					case 'i':
						if (!editando_en_zona_ascii) valor_xor ^= 255;
					break;

					case 't':
						if (!editando_en_zona_ascii) {
							menu_hexdump_edit_mode ^= 1;
							menu_espera_no_tecla();
							tecla=0; //para no enviar dicha tecla al editor
						}

						//Si zona de filemem
						if (menu_hexdump_edit_mode && menu_debug_memory_zone==MEMORY_ZONE_NUM_FILE_ZONE) {
							menu_debug_hexdump_aviso_edit_filezone(&ventana);				
						}
					break;	

					case 'u':
						//Ver info subzonas
						menu_debug_hexdump_info_subzones();
					break;				

					//case 'l':
					//	menu_debug_hex_shows_inves_low_ram.v ^=1;
					//break;

					case 'z':

						if (!editando_en_zona_ascii) {
							menu_debug_change_memory_zone();
							asked_about_writing_rom=0;
						}

						break;

					//Salir con ESC si no es modo edit
					case 2:
						if (menu_hexdump_edit_mode) {
							menu_hexdump_edit_mode=0;
						}
						else salir=1;
					break;

					//Enter tambien sale de modo edit
					case 13:
						if (menu_hexdump_edit_mode) menu_hexdump_edit_mode=0;
					break;



				}

				//Y ahora para el caso de edit_mode y pulsar tecla hexa o ascii segun la zona
				int editar_byte=0;
				if (menu_hexdump_edit_mode) {
					//Para la zona ascii
					if (cursor_en_zona_ascii && tecla>=32 && tecla<=126) editar_byte=1; 

					//Para la zona hexa
					if (
						!cursor_en_zona_ascii && 
						( (tecla>='0' && tecla<='9') || (tecla>='a' && tecla<='f') )
						) {
						editar_byte=1; 
					}
				}

				//Ver si vamos a editar en zona de rom
				if (editar_byte) {
					int attribute_zone;
					//Si zona por defecto mapped memory, asumimos lectura/escritura
					if (menu_debug_memory_zone==-1) attribute_zone=3;
					else machine_get_memory_zone_attrib(menu_debug_memory_zone, &attribute_zone);

					//printf ("Attrib zone %d asked %d\n",attribute_zone,asked_about_writing_rom);

					//Attrib: bit 0: read, bit 1: write
					//Si no tiene atributo de escritura y no se ha pedido antes si se quiere escribir en rom
					if ( (attribute_zone&2)==0 && !asked_about_writing_rom) {
						if (menu_confirm_yesno_texto("Memory zone is ROM","Sure you want to edit?")==0) {
							editar_byte=0;
						}
						else {
							asked_about_writing_rom=1;
						}

						//Volver a dibujar ventana, pues se ha borrado al pregutar confirmacion
						//menu_debug_hexdump_ventana();
						zxvision_draw_window(&ventana);
					}

 
				}


				//asked_about_writing_rom

				if (editar_byte) {
						//Obtener direccion puntero
						menu_z80_moto_int direccion_cursor=menu_debug_hexdump_direccion;

						//int si_zona_hexa=0; //en zona hexa o ascii
						//if (menu_hexdump_edit_position_x<bytes_por_linea*2) si_zona_hexa=1;


						if (!cursor_en_zona_ascii) {
							//Sumar x (cada dos, una posicion)
							direccion_cursor +=menu_hexdump_edit_position_x/2;
						}
						else {
							int indice_hasta_ascii=menu_hexdump_bytes_por_linea*2+1; //el hexa y el espacio
							direccion_cursor +=menu_hexdump_edit_position_x-indice_hasta_ascii;
						}

						//Sumar y. 
						direccion_cursor +=menu_hexdump_edit_position_y*menu_hexdump_bytes_por_linea;

						//Ajustar direccion a zona memoria
						direccion_cursor=adjust_address_memory_size(direccion_cursor);

						//TODO: ver si se sale de tamanyo zona memoria

						//printf ("Direccion edicion: %X\n",direccion_cursor);

						//Obtenemos byte en esa posicion
						z80_byte valor_leido=menu_debug_get_mapped_byte(direccion_cursor);


						//Estamos en zona hexa o ascii

						if (!cursor_en_zona_ascii) {
							//printf ("Zona hexa\n");
							//Zona hexa

							//Obtener valor nibble
							z80_byte valor_nibble;

							if (tecla>='0' && tecla<='9') valor_nibble=tecla-'0';
							else valor_nibble=tecla-'a'+10;

							//Ver si par o impar
							if ( (menu_hexdump_edit_position_x %2) ==0) {
								//par. alterar nibble alto
								valor_leido=(valor_leido&0x0F) | (valor_nibble<<4);
							}

							else {
								//impar. alterar nibble bajo
								valor_leido=(valor_leido&0xF0) | valor_nibble;
							}
						}

						else {
							//printf ("Zona ascii\n");
							valor_leido=tecla;

							//En el caso de zx80/81

							if (menu_debug_hexdump_with_ascii_modo_ascii==1) valor_leido=ascii_to_zx80(valor_leido);
							if (menu_debug_hexdump_with_ascii_modo_ascii==2) valor_leido=ascii_to_zx81(valor_leido);
						}



						//Escribimos valor
						menu_debug_write_mapped_byte(direccion_cursor,valor_leido);

						//Y mover cursor a la derecha
						menu_debug_hexdump_edit_cursor_derecha(1);

						//Si se llega a detecha de hexa o ascii, saltar linea

					
				}	


        } while (salir==0);

	cls_menu_overlay();
	zxvision_destroy_window(&ventana);
	

}



//Entrada seleccionada
int adventure_keyboard_selected_item=0;

//Posicion dentro del string
int adventure_keyboard_index_selected_item=0;

//z80_bit menu_osd_adventure_sending_keys={0};




int adventure_keyboard_pending_send_final_spc=1;

void menu_osd_adventure_kb_press_key_variable(char letra)
{
	if (letra==0) return; //pequenyo bug: si acaba texto con ~~ no se abrira luego de nuevo el menu. Bug???

	//printf ("Pulsar tecla entrada %d indice en entrada: %d letra: %c\n",adventure_keyboard_selected_item,adventure_keyboard_index_selected_item,letra);
	//osd_adv_kbd_list
	debug_printf (VERBOSE_DEBUG,"Pressing key %c of word %s",letra,osd_adv_kbd_list[adventure_keyboard_selected_item]);

	//Espacio no la gestiona esta funcion de convert_numeros_...
	if (letra==' ') util_set_reset_key(UTIL_KEY_SPACE,1);
	//else convert_numeros_letras_puerto_teclado_continue(letra,1);
	else ascii_to_keyboard_port(letra);

	//Lanzar pulsar tecla 
	timer_on_screen_adv_key=adventure_keyboard_key_length; 
}

void menu_osd_adventure_kb_press_key(void)
{

	//Aunque el usuario haya puesto alguna mayuscula, metemos minusculas
	char letra;

	//Ignorar ~~

	do {
		letra=letra_minuscula(osd_adv_kbd_list[adventure_keyboard_selected_item][adventure_keyboard_index_selected_item]);
		if (letra=='~') adventure_keyboard_index_selected_item++;
	} while (letra=='~' && letra!=0);

	menu_osd_adventure_kb_press_key_variable(letra);

	/*if (letra==0) return; //pequenyo bug: si acaba texto con ~~ no se abrira luego de nuevo el menu. Bug???

	//printf ("Pulsar tecla entrada %d indice en entrada: %d letra: %c\n",adventure_keyboard_selected_item,adventure_keyboard_index_selected_item,letra);
	//osd_adv_kbd_list
	debug_printf (VERBOSE_DEBUG,"Pressing key %c of word %s",letra,osd_adv_kbd_list[adventure_keyboard_selected_item]);

	//Espacio no la gestiona esta funcion de convert_numeros_...
	if (letra==' ') util_set_reset_key(UTIL_KEY_SPACE,1);
	//else convert_numeros_letras_puerto_teclado_continue(letra,1);
	else ascii_to_keyboard_port(letra);

	//Lanzar pulsar tecla 
	timer_on_screen_adv_key=adventure_keyboard_key_length; */

}


void menu_osd_adventure_keyboard_action(MENU_ITEM_PARAMETERS)
{
	//printf ("opcion seleccionada: %d\n",valor_opcion);
	adventure_keyboard_selected_item=valor_opcion;
	adventure_keyboard_index_selected_item=0;
	adventure_keyboard_pending_send_final_spc=1;


	//Estamos enviando teclas
	//menu_osd_adventure_sending_keys.v=1;

	menu_osd_adventure_kb_press_key();
}

//Retorno desde el core
void menu_osd_adventure_keyboard_next(void)
{

	//if (menu_osd_adventure_sending_keys.v==0 return;

	//Si final de string
	adventure_keyboard_index_selected_item++;
	if (osd_adv_kbd_list[adventure_keyboard_selected_item][adventure_keyboard_index_selected_item]==0) {
		//printf ("Fin texto\n");
		
		//Ver si habia que enviar un espacio al final
		if (adventure_keyboard_send_final_spc && adventure_keyboard_pending_send_final_spc) {
			menu_osd_adventure_kb_press_key_variable(' ');
			adventure_keyboard_pending_send_final_spc=0;
		}
		else {
			//En este caso reabrir el menu
			menu_osd_adventure_keyboard(0);
			return;
		}
	}

	//Siguiente tecla
	else menu_osd_adventure_kb_press_key();
}


#define ADVENTURE_KB_X 0
#define ADVENTURE_KB_Y 0
#define ADVENTURE_KB_ANCHO 32
#define ADVENTURE_KB_ALTO 24

//maximo de alto total admitido para la ventana
#define ADVENTURE_KB_MAX_TOTAL_HEIGHT 500

//conservar valor de scroll ultimo para que cuando listado sea grande,
//poder conservar ultima posicion
int menu_osd_advkb_last_offset_y=0;

void menu_osd_adventure_keyboard(MENU_ITEM_PARAMETERS)
{

	//Si estamos enviando teclas
	//if (menu_osd_adventure_sending_keys.v) {
	//	menu_osd_adventure_keyboard_next();
	//	return;
	//}

	//Si lista vacia, error
	if (osd_adv_kbd_defined==0) {
		debug_printf (VERBOSE_ERR,"Empty list");
		return;
	}

	//Si estamos enviando teclas, desactivar
	timer_on_screen_adv_key=0;



 	menu_espera_no_tecla();
	menu_reset_counters_tecla_repeticion();		

	zxvision_window ventana;

	zxvision_new_window(&ventana,ADVENTURE_KB_X,ADVENTURE_KB_Y,ADVENTURE_KB_ANCHO,ADVENTURE_KB_ALTO,
							ADVENTURE_KB_ANCHO-1,ADVENTURE_KB_MAX_TOTAL_HEIGHT,"OSD Adventure Keyboard");
	zxvision_draw_window(&ventana);		


       
        menu_item *array_menu_osd_adventure_keyboard;
        menu_item item_seleccionado;
        int retorno_menu;
        do {

		int initial_test; //si es 1, haremos el calculo inicial del alto

		int alto_ventana=ADVENTURE_KB_ALTO;
		int y_ventana=ADVENTURE_KB_Y;

		for (initial_test=1;initial_test>=0;initial_test--) {


          //Hay que redibujar la ventana desde este bucle
          if (!initial_test) {
			  //menu_dibuja_ventana(ADVENTURE_KB_X,y_ventana,ADVENTURE_KB_ANCHO,alto_ventana,"OSD Adventure Keyboard");
			  zxvision_set_y_position(&ventana,y_ventana);
			  zxvision_set_visible_height(&ventana,alto_ventana);
			  
			  //Alteramos alto total para que coincida con alto ventana (siempre que sea menor que el alto actual)
			  //si fuese mayor el alto, estariamos necesitando mas memoria y seria un problema
			  //esto es un poco feo realmente, pero bueno, al reducir el tamaño no hay problema de que nos salgamos de la memoria
			  int current_height=ventana.total_height;
			  int desired_height=alto_ventana-2;

			  if (desired_height<current_height) {
				  ventana.total_height=desired_height;
			  }
		  }


        //Como no sabemos cual sera el item inicial, metemos este sin asignar, que se sobreescribe en el siguiente menu_add_item_menu
        menu_add_item_menu_inicial(&array_menu_osd_adventure_keyboard,"",MENU_OPCION_UNASSIGNED,NULL,NULL);

	//if (osd_adv_kbd_list[adventure_keyboard_selected_item][adventure_keyboard_index_selected_item]==0) {
	//osd_adv_kbd_defined
		int i;
		int last_x=1;
		int last_y=0;
		int salir=0;

		//Asignar hotkeys, segun si se han asignado antes o no
		//int hotkeys_assigned[26]; //de la A a la Z
		//for (i=0;i<26;i++) hotkeys_assigned[i]=0;
		
		
		for (i=0;i<osd_adv_kbd_defined && !salir;i++) {
			int longitud_texto=strlen(osd_adv_kbd_list[i])+1; //Espacio para la entrada y 1 espacio
			if (last_x+longitud_texto>ADVENTURE_KB_ANCHO) {
				last_x=1;
				last_y++; 
			}

			//controlar maximo de alto
			if (last_y>=ADVENTURE_KB_MAX_TOTAL_HEIGHT) {
				debug_printf (VERBOSE_DEBUG,"Reached maximum window height");
				last_y--;
				salir=1;
			}

			else {
				//Si es cadena vacia, ignorarla. No deberia pasar pues se debe denegar desde donde se lee la configuracion, pero por si acaso
				if (osd_adv_kbd_list[i][0]==0) {
					debug_printf (VERBOSE_DEBUG,"Null string at %d",i);
				}

				else {

					int tiene_hotkey=0;

					char texto_opcion[64];
					strcpy(texto_opcion,osd_adv_kbd_list[i]);

					char hotkey;
					
					//Caracter de hotkey. Crearlo automaticamente
					//hotkey=letra_minuscula(osd_adv_kbd_list[i][0]);

					//Caracter de hotkey. Dejar que el usuario lo escriba en la cadena de texto. Ver si dicha cadena lo tiene

					int j;
					for (j=0;texto_opcion[j];j++) {
						if (texto_opcion[j]=='~' && texto_opcion[j+1]=='~') {
							//Si hay letra detras
							hotkey=letra_minuscula(texto_opcion[j+2]);
							if (hotkey) tiene_hotkey=1;
						}
					}
					

				    menu_add_item_menu_format(array_menu_osd_adventure_keyboard,MENU_OPCION_NORMAL,menu_osd_adventure_keyboard_action,NULL,texto_opcion);
        		    menu_add_item_menu_tabulado(array_menu_osd_adventure_keyboard,last_x,last_y);
					menu_add_item_menu_valor_opcion(array_menu_osd_adventure_keyboard,i);
					//printf ("Agregando palabra %s en %d,%d\n",texto_opcion,last_x,last_y);

					if (tiene_hotkey) {
						menu_add_item_menu_shortcut(array_menu_osd_adventure_keyboard,hotkey);
						longitud_texto -=2;
					}

				}

				last_x+=longitud_texto;
			}

		}

		//Recalcular alto, y_inivial
		//del alto, se pierden 2 siempre
		//si tuvieramos el maximo de y, valdria 21. Y el maximo de alto es 24
		//printf ("ultima y: %d\n",last_y);
		alto_ventana=last_y+3;
		y_ventana=12-alto_ventana/2;
		if (y_ventana<0) y_ventana=0;	


		}


		//Recuperamos antiguo offset de ventana
		zxvision_set_offset_y(&ventana,menu_osd_advkb_last_offset_y);


		//Nombre de ventana solo aparece en el caso de stdout
        retorno_menu=menu_dibuja_menu(&osd_adventure_keyboard_opcion_seleccionada,&item_seleccionado,array_menu_osd_adventure_keyboard,"OSD Adventure KB" );


	//En caso de menus tabulados, es responsabilidad de este de borrar la ventana
        cls_menu_overlay();
                if ((item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu>=0) {
                        //llamamos por valor de funcion
                        if (item_seleccionado.menu_funcion!=NULL) {
				//printf ("Item seleccionado: %d\n",item_seleccionado.valor_opcion);
                                //printf ("actuamos por funcion\n");

	                        salir_todos_menus=1;

                                item_seleccionado.menu_funcion(item_seleccionado.valor_opcion);
				//En caso de menus tabulados, es responsabilidad de este de borrar la ventana
                                
                        }
                }

        } while ( (item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu!=MENU_RETORNO_ESC && !salir_todos_menus);


		//Guardamos offset de ventana actual
		menu_osd_advkb_last_offset_y=ventana.offset_y;

        cls_menu_overlay();
		//menu_espera_no_tecla();

		//menu_abierto=1;
		//Si con control de joystick se ha salido con tecla ESCMenu, esa tecla de joystick lo que hace es ESC
		//pero luego fuerza a abrir el menu de nuevo. Por tanto, decimos que no hay que abrir menu
		menu_event_open_menu.v=0;

		//printf ("en final de funcion\n");
		zxvision_destroy_window(&ventana);

}









//Usado dentro del overlay de tsconf_dma
//int menu_tsconf_dma_valor_contador_segundo_anterior;

zxvision_window *menu_debug_dma_tsconf_zxuno_overlay_window;


void menu_debug_dma_tsconf_zxuno_overlay(void)
{

    normal_overlay_texto_menu();

    int linea=0;
   

    
    	//mostrarlos siempre a cada refresco
    menu_speech_tecla_pulsada=1; //Si no, envia continuamente todo ese texto a speech

	char texto_dma[33];

	if (datagear_dma_emulation.v) {
		//NOTA: Si se activa datagear, no se vera si hay dma de tsconf o zxuno
		z80_int dma_port_a=value_8_to_16(datagear_port_a_start_addr_high,datagear_port_a_start_addr_low);
		z80_int dma_port_b=value_8_to_16(datagear_port_b_start_addr_high,datagear_port_b_start_addr_low);

		z80_int dma_len=value_8_to_16(datagear_block_length_high,datagear_block_length_low);	

		sprintf (texto_dma,"Port A:      %04XH",dma_port_a);
		//menu_escribe_linea_opcion(linea++,-1,1,texto_dma);
		zxvision_print_string_defaults_fillspc(menu_debug_dma_tsconf_zxuno_overlay_window,1,linea++,texto_dma);

		sprintf (texto_dma,"Port B:      %04XH",dma_port_b);
		//menu_escribe_linea_opcion(linea++,-1,1,texto_dma);
		zxvision_print_string_defaults_fillspc(menu_debug_dma_tsconf_zxuno_overlay_window,1,linea++,texto_dma);

		sprintf (texto_dma,"Lenght:      %5d",dma_len);
		//menu_escribe_linea_opcion(linea++,-1,1,texto_dma);
		zxvision_print_string_defaults_fillspc(menu_debug_dma_tsconf_zxuno_overlay_window,1,linea++,texto_dma);

		if (datagear_wr0 & 4) sprintf (texto_dma,"Port A->B");
		else sprintf (texto_dma,"Port B->A");

		//menu_escribe_linea_opcion(linea++,-1,1,texto_dma);
		zxvision_print_string_defaults_fillspc(menu_debug_dma_tsconf_zxuno_overlay_window,1,linea++,texto_dma);



		char access_type[20];

        if (datagear_wr1 & 8) sprintf (access_type,"I/O"); 
		else sprintf (access_type,"Memory");

		if ( (datagear_wr1 & 32) == 0 ) {
            if (datagear_wr1 & 16) sprintf (texto_dma,"Port A++. %s",access_type);
            else sprintf (texto_dma,"Port A--. %s",access_type);
        }
		else sprintf (texto_dma,"Port A fixed. %s",access_type);
		//menu_escribe_linea_opcion(linea++,-1,1,texto_dma);
		zxvision_print_string_defaults_fillspc(menu_debug_dma_tsconf_zxuno_overlay_window,1,linea++,texto_dma);


        if (datagear_wr2 & 8) sprintf (access_type,"I/O"); 
		else sprintf (access_type,"Memory");

		if ( (datagear_wr2 & 32) == 0 ) {
            if (datagear_wr2 & 16) sprintf (texto_dma,"Port B++. %s",access_type);
            else sprintf (texto_dma,"Port B--. %s",access_type);
        }
		else sprintf (texto_dma,"Port B fixed. %s",access_type);
		//menu_escribe_linea_opcion(linea++,-1,1,texto_dma);	
		zxvision_print_string_defaults_fillspc(menu_debug_dma_tsconf_zxuno_overlay_window,1,linea++,texto_dma);

		//WR4. Bits D6 D5:
		//#       0   0 = Byte mode -> Do not use (Behaves like Continuous mode, Byte mode on Z80 DMA)
		//#       0   1 = Continuous mode
		//#       1   0 = Burst mode
		//#       1   1 = Do not use

		z80_byte modo_transferencia=(datagear_wr4>>5)&3;
		if (modo_transferencia==0) 		sprintf (texto_dma,"Mode: Byte mode");
		else if (modo_transferencia==1) sprintf (texto_dma,"Mode: Continuous");
		else if (modo_transferencia==2) sprintf (texto_dma,"Mode: Burst");
		else 							sprintf (texto_dma,"Mode: Do not use");

		//menu_escribe_linea_opcion(linea++,-1,1,texto_dma);	
		zxvision_print_string_defaults_fillspc(menu_debug_dma_tsconf_zxuno_overlay_window,1,linea++,texto_dma);



	}

	else {

	if (MACHINE_IS_TSCONF) {
		//Construimos 16 valores posibles segun rw (bit bajo) y ddev (bits altos)
		int dma_type=debug_tsconf_dma_ddev*2+debug_tsconf_dma_rw;
						//18 maximo el tipo
						//  012345678901234567890123
						//24. mas dos de margen banda y banda: 26
		sprintf (texto_dma,"Type: %s",tsconf_dma_types[dma_type]);
		//menu_escribe_linea_opcion(linea++,-1,1,texto_dma);
		zxvision_print_string_defaults_fillspc(menu_debug_dma_tsconf_zxuno_overlay_window,1,linea++,texto_dma);

		sprintf (texto_dma,"Source:      %06XH",debug_tsconf_dma_source);
		//menu_escribe_linea_opcion(linea++,-1,1,texto_dma);
		zxvision_print_string_defaults_fillspc(menu_debug_dma_tsconf_zxuno_overlay_window,1,linea++,texto_dma);

		sprintf (texto_dma,"Destination: %06XH",debug_tsconf_dma_destination);
		//menu_escribe_linea_opcion(linea++,-1,1,texto_dma);
		zxvision_print_string_defaults_fillspc(menu_debug_dma_tsconf_zxuno_overlay_window,1,linea++,texto_dma);

		sprintf (texto_dma,"Burst lenght: %3d",debug_tsconf_dma_burst_length);
		//menu_escribe_linea_opcion(linea++,-1,1,texto_dma);
		zxvision_print_string_defaults_fillspc(menu_debug_dma_tsconf_zxuno_overlay_window,1,linea++,texto_dma);

		sprintf (texto_dma,"Burst number: %3d",debug_tsconf_dma_burst_number);
		//menu_escribe_linea_opcion(linea++,-1,1,texto_dma);
		zxvision_print_string_defaults_fillspc(menu_debug_dma_tsconf_zxuno_overlay_window,1,linea++,texto_dma);

						//Maximo 25
		sprintf (texto_dma,"Align: %s %s",(debug_tsconf_dma_s_align ? "Source" : "      "),(debug_tsconf_dma_d_align ? "Destination" : "") );
		//menu_escribe_linea_opcion(linea++,-1,1,texto_dma);
		zxvision_print_string_defaults_fillspc(menu_debug_dma_tsconf_zxuno_overlay_window,1,linea++,texto_dma);

		sprintf (texto_dma,"Align size: %d",(debug_tsconf_dma_addr_align_size+1)*256);
		//menu_escribe_linea_opcion(linea++,-1,1,texto_dma);
		zxvision_print_string_defaults_fillspc(menu_debug_dma_tsconf_zxuno_overlay_window,1,linea++,texto_dma);

	}

	if (MACHINE_IS_ZXUNO) {
		z80_byte dma_ctrl=zxuno_ports[0xa0];
		z80_byte dma_type=(dma_ctrl & (4+8))>>2;
		z80_byte dma_mode=dma_ctrl & 3;

		z80_int dma_src=value_8_to_16(zxuno_dmareg[0][1],zxuno_dmareg[0][0]);
		z80_int dma_dst=value_8_to_16(zxuno_dmareg[1][1],zxuno_dmareg[1][0]);
		z80_int dma_pre=value_8_to_16(zxuno_dmareg[2][1],zxuno_dmareg[2][0]);
		z80_int dma_len=value_8_to_16(zxuno_dmareg[3][1],zxuno_dmareg[3][0]);	
		z80_int dma_prob=value_8_to_16(zxuno_dmareg[4][1],zxuno_dmareg[4][0]);		
		z80_byte dma_stat=zxuno_ports[0xa6];

		sprintf (texto_dma,"Type: %s",zxuno_dma_types[dma_type]);
		//menu_escribe_linea_opcion(linea++,-1,1,texto_dma);
		zxvision_print_string_defaults_fillspc(menu_debug_dma_tsconf_zxuno_overlay_window,1,linea++,texto_dma);

		sprintf (texto_dma,"Mode: %s",zxuno_dma_modes[dma_mode]);
		//menu_escribe_linea_opcion(linea++,-1,1,texto_dma);		
		zxvision_print_string_defaults_fillspc(menu_debug_dma_tsconf_zxuno_overlay_window,1,linea++,texto_dma);

		sprintf (texto_dma,"Source:      %04XH",dma_src);
		//menu_escribe_linea_opcion(linea++,-1,1,texto_dma);
		zxvision_print_string_defaults_fillspc(menu_debug_dma_tsconf_zxuno_overlay_window,1,linea++,texto_dma);

		sprintf (texto_dma,"Destination: %04XH",dma_dst);
		//menu_escribe_linea_opcion(linea++,-1,1,texto_dma);
		zxvision_print_string_defaults_fillspc(menu_debug_dma_tsconf_zxuno_overlay_window,1,linea++,texto_dma);

		sprintf (texto_dma,"Lenght:      %5d",dma_len);
		//menu_escribe_linea_opcion(linea++,-1,1,texto_dma);
		zxvision_print_string_defaults_fillspc(menu_debug_dma_tsconf_zxuno_overlay_window,1,linea++,texto_dma);

		sprintf (texto_dma,"Preescaler:  %5d",dma_pre);
		//menu_escribe_linea_opcion(linea++,-1,1,texto_dma);
		zxvision_print_string_defaults_fillspc(menu_debug_dma_tsconf_zxuno_overlay_window,1,linea++,texto_dma);

		char prob_type[10];
		if (dma_ctrl&16) strcpy(prob_type,"dst");
		else strcpy(prob_type,"src");

		sprintf (texto_dma,"Prob: (%s)  %04XH",prob_type,dma_prob);
		//menu_escribe_linea_opcion(linea++,-1,1,texto_dma);		
		zxvision_print_string_defaults_fillspc(menu_debug_dma_tsconf_zxuno_overlay_window,1,linea++,texto_dma);

		sprintf (texto_dma,"Stat:          %02XH",dma_stat);
		//menu_escribe_linea_opcion(linea++,-1,1,texto_dma);			
		zxvision_print_string_defaults_fillspc(menu_debug_dma_tsconf_zxuno_overlay_window,1,linea++,texto_dma);

	}

	}

	zxvision_draw_window_contents(menu_debug_dma_tsconf_zxuno_overlay_window);
}






void menu_debug_dma_tsconf_zxuno_disable(MENU_ITEM_PARAMETERS)
{
	if (datagear_dma_emulation.v) datagear_dma_is_disabled.v ^=1;

	else {
		if (MACHINE_IS_TSCONF) tsconf_dma_disabled.v ^=1;
		if (MACHINE_IS_ZXUNO) zxuno_dma_disabled.v ^=1;
	}
}


void menu_debug_dma_tsconf_zxuno(MENU_ITEM_PARAMETERS)
{
 	menu_espera_no_tecla();
	menu_reset_counters_tecla_repeticion();		





	char texto_ventana[33];
	//por defecto por si acaso
	strcpy(texto_ventana,"DMA");
	int alto=11;


	if (MACHINE_IS_ZXUNO) {
		strcpy(texto_ventana,"ZXUNO DMA");
		alto++;
	}

	if (MACHINE_IS_TSCONF) strcpy(texto_ventana,"TSConf DMA");

	if (datagear_dma_emulation.v) strcpy(texto_ventana,"Datagear DMA");	


	//menu_dibuja_ventana(2,6,27,alto,texto_ventana);
	zxvision_window ventana;

	zxvision_new_window(&ventana,2,6,27,alto,
							27-1,alto-2,texto_ventana);
	zxvision_draw_window(&ventana);			



    //Cambiamos funcion overlay de texto de menu
	set_menu_overlay_function(menu_debug_dma_tsconf_zxuno_overlay);

	menu_debug_dma_tsconf_zxuno_overlay_window=&ventana; //Decimos que el overlay lo hace sobre la ventana que tenemos aqui			



	menu_item *array_menu_debug_dma_tsconf_zxuno;
        menu_item item_seleccionado;
        int retorno_menu;
        do {

        			
            //Hay que redibujar la ventana desde este bucle
            //menu_debug_dma_tsconf_zxuno_dibuja_ventana();

	

			int lin=8;

			

			int condicion_dma_disabled=tsconf_dma_disabled.v;


			if (MACHINE_IS_ZXUNO) {
				lin++;	
				condicion_dma_disabled=zxuno_dma_disabled.v;
			}

			if (datagear_dma_emulation.v) condicion_dma_disabled=datagear_dma_is_disabled.v;
		
				menu_add_item_menu_inicial_format(&array_menu_debug_dma_tsconf_zxuno,MENU_OPCION_NORMAL,menu_debug_dma_tsconf_zxuno_disable,NULL,"~~DMA: %s",
					(condicion_dma_disabled ? "Disabled" : "Enabled ") );  //Enabled acaba con espacio para borrar rastro de texto "Disabled"
				menu_add_item_menu_shortcut(array_menu_debug_dma_tsconf_zxuno,'d');
				menu_add_item_menu_ayuda(array_menu_debug_dma_tsconf_zxuno,"Disable DMA");
				menu_add_item_menu_tabulado(array_menu_debug_dma_tsconf_zxuno,1,lin);




		//Nombre de ventana solo aparece en el caso de stdout
                retorno_menu=menu_dibuja_menu(&debug_tsconf_dma_opcion_seleccionada,&item_seleccionado,array_menu_debug_dma_tsconf_zxuno,"TSConf DMA" );


	//En caso de menus tabulados, es responsabilidad de este de borrar la ventana
	cls_menu_overlay();
                if ((item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu>=0) {
                        //llamamos por valor de funcion
                        if (item_seleccionado.menu_funcion!=NULL) {
                                //printf ("actuamos por funcion\n");
                                item_seleccionado.menu_funcion(item_seleccionado.valor_opcion);
				//En caso de menus tabulados, es responsabilidad de este de borrar la ventana
                                
                        }
                }

        } while ( (item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu!=MENU_RETORNO_ESC && !salir_todos_menus);



       //restauramos modo normal de texto de menu
       set_menu_overlay_function(normal_overlay_texto_menu);


        cls_menu_overlay();

	//En caso de menus tabulados, es responsabilidad de este de liberar ventana
	zxvision_destroy_window(&ventana);				

}




int menu_tsconf_layer_valor_contador_segundo_anterior;

char *menu_tsconf_layer_aux_usedunused_used="In use";
char *menu_tsconf_layer_aux_usedunused_unused="Unused";

char *menu_tsconf_layer_aux_usedunused(int value)
{
	if (value) return menu_tsconf_layer_aux_usedunused_used;
	else return menu_tsconf_layer_aux_usedunused_unused;
}


zxvision_window *menu_tsconf_layer_overlay_window;

void menu_tsconf_layer_overlay_mostrar_texto(void)
{
 int linea;

    linea=0;

    
        //mostrarlos siempre a cada refresco

                char texto_layer[33];

				if (MACHINE_IS_TSCONF) {

				//menu_escribe_linea_opcion(linea,-1,1,"Border: ");
				zxvision_print_string_defaults_fillspc(menu_tsconf_layer_overlay_window,1,linea,"Border: ");
				linea +=3;

                sprintf (texto_layer,"ULA:       %s",menu_tsconf_layer_aux_usedunused(tsconf_if_ula_enabled()));
                //menu_escribe_linea_opcion(linea,-1,1,texto_layer);
				zxvision_print_string_defaults_fillspc(menu_tsconf_layer_overlay_window,1,linea,texto_layer);
				linea +=3;

                sprintf (texto_layer,"Sprites 0: %s",menu_tsconf_layer_aux_usedunused(tsconf_if_sprites_enabled()));
                //menu_escribe_linea_opcion(linea,-1,1,texto_layer);	
				zxvision_print_string_defaults_fillspc(menu_tsconf_layer_overlay_window,1,linea,texto_layer);
				linea +=3;		

				sprintf (texto_layer,"Tiles 0:   %s",menu_tsconf_layer_aux_usedunused(tsconf_if_tiles_zero_enabled()));
                //menu_escribe_linea_opcion(linea,-1,1,texto_layer);
				zxvision_print_string_defaults_fillspc(menu_tsconf_layer_overlay_window,1,linea,texto_layer);
				linea +=3;	

                sprintf (texto_layer,"Sprites 1: %s",menu_tsconf_layer_aux_usedunused(tsconf_if_sprites_enabled()));
                //menu_escribe_linea_opcion(linea,-1,1,texto_layer);	
				zxvision_print_string_defaults_fillspc(menu_tsconf_layer_overlay_window,1,linea,texto_layer);
				linea +=3;	

		    	sprintf (texto_layer,"Tiles 1:   %s",menu_tsconf_layer_aux_usedunused(tsconf_if_tiles_one_enabled()));
                //menu_escribe_linea_opcion(linea,-1,1,texto_layer);
				zxvision_print_string_defaults_fillspc(menu_tsconf_layer_overlay_window,1,linea,texto_layer);
				linea +=3;

                sprintf (texto_layer,"Sprites 2: %s",menu_tsconf_layer_aux_usedunused(tsconf_if_sprites_enabled()));
                //menu_escribe_linea_opcion(linea,-1,1,texto_layer);	
				zxvision_print_string_defaults_fillspc(menu_tsconf_layer_overlay_window,1,linea,texto_layer);
				linea +=3;		
				}

				if (MACHINE_IS_TBBLUE) {
	                sprintf (texto_layer,"ULA:       %s",menu_tsconf_layer_aux_usedunused(tbblue_if_ula_is_enabled()) ); 
    	            //menu_escribe_linea_opcion(linea,-1,1,texto_layer);
					zxvision_print_string_defaults_fillspc(menu_tsconf_layer_overlay_window,1,linea,texto_layer);
					linea +=3;

	                sprintf (texto_layer,"Tiles:     %s",menu_tsconf_layer_aux_usedunused(tbblue_if_tilemap_enabled()) ); 
    	            //menu_escribe_linea_opcion(linea,-1,1,texto_layer);
					zxvision_print_string_defaults_fillspc(menu_tsconf_layer_overlay_window,1,linea,texto_layer);
					linea +=3;			

					zxvision_print_string_defaults(menu_tsconf_layer_overlay_window,1,linea,"ULA&Tiles:");
					linea +=2;									

                	sprintf (texto_layer,"Sprites:   %s",menu_tsconf_layer_aux_usedunused(tbblue_if_sprites_enabled() ));
                	//menu_escribe_linea_opcion(linea,-1,1,texto_layer);	
					zxvision_print_string_defaults_fillspc(menu_tsconf_layer_overlay_window,1,linea,texto_layer);
					linea +=3;		

					sprintf (texto_layer,"Layer 2:   %s",menu_tsconf_layer_aux_usedunused(tbblue_is_active_layer2() ) );
    	            //menu_escribe_linea_opcion(linea,-1,1,texto_layer);
					zxvision_print_string_defaults_fillspc(menu_tsconf_layer_overlay_window,1,linea,texto_layer);
					linea +=3;						


					//Layer priorities

					z80_byte prio=tbblue_get_layers_priorities();
					sprintf (texto_layer,"Priorities: (%d)",prio);
					//menu_escribe_linea_opcion(linea++,-1,1,texto_layer);
					zxvision_print_string_defaults_fillspc(menu_tsconf_layer_overlay_window,1,linea++,texto_layer);

				
					int i;
					for (i=0;i<3;i++) {
						char nombre_capa[32];
						strcpy(nombre_capa,tbblue_get_string_layer_prio(i,prio) );
						//if (strcmp(nombre_capa,"ULA&Tiles")) strcpy(nombre_capa,"  ULA  "); //meter espacios para centrarlo
						//las otras capas son "Sprites" y "Layer 2" y ocupan lo mismo

													//     Sprites
													//    ULA&Tiles
						if (i!=2) strcpy (texto_layer,"|---------------|");
						else      strcpy (texto_layer,"v---------------v");

						//Centrar el nombre de capa
						int longitud_medio=strlen(nombre_capa)/2;
						int medio=strlen(texto_layer)/2;
						int pos=medio-longitud_medio;
						if (pos<0) pos=0;

						//Meter texto centrado y quitar 0 del final
						strcpy(&texto_layer[pos],nombre_capa);

						int final=strlen(texto_layer);
						texto_layer[final]='-';

						//menu_escribe_linea_opcion(linea++,-1,1,texto_layer);
						zxvision_print_string_defaults_fillspc(menu_tsconf_layer_overlay_window,1,linea++,texto_layer);

					}
				
				}			


         



}



void menu_tsconf_layer_overlay(void)
{

    normal_overlay_texto_menu();

 	menu_speech_tecla_pulsada=1; //Si no, envia continuamente todo ese texto a speech

 
    //esto hara ejecutar esto 2 veces por segundo
    if ( ((contador_segundo%500) == 0 && menu_tsconf_layer_valor_contador_segundo_anterior!=contador_segundo) || menu_multitarea==0) {

        menu_tsconf_layer_valor_contador_segundo_anterior=contador_segundo;
        //printf ("Refrescando. contador_segundo=%d\n",contador_segundo);
       

		menu_tsconf_layer_overlay_mostrar_texto();
		zxvision_draw_window_contents(menu_tsconf_layer_overlay_window);

    }
}


void menu_tsconf_layer_settings_ula(MENU_ITEM_PARAMETERS)
{
	tsconf_force_disable_layer_ula.v ^=1;
}


void menu_tsconf_layer_settings_sprites_zero(MENU_ITEM_PARAMETERS)
{
	tsconf_force_disable_layer_sprites_zero.v ^=1;
}

void menu_tsconf_layer_settings_sprites_one(MENU_ITEM_PARAMETERS)
{
	tsconf_force_disable_layer_sprites_one.v ^=1;
}

void menu_tsconf_layer_settings_sprites_two(MENU_ITEM_PARAMETERS)
{
	tsconf_force_disable_layer_sprites_two.v ^=1;
}

void menu_tsconf_layer_settings_tiles_zero(MENU_ITEM_PARAMETERS)
{
	tsconf_force_disable_layer_tiles_zero.v ^=1;
}

void menu_tsconf_layer_settings_tiles_one(MENU_ITEM_PARAMETERS)
{
	tsconf_force_disable_layer_tiles_one.v ^=1;
}

void menu_tsconf_layer_settings_border(MENU_ITEM_PARAMETERS)
{
	tsconf_force_disable_layer_border.v ^=1;
}

/*
extern z80_bit tbblue_force_disable_layer_ula;
extern z80_bit tbblue_force_disable_layer_sprites;
extern z80_bit tbblue_force_disable_layer_layer_two;
*/

void menu_tbblue_layer_settings_sprites(MENU_ITEM_PARAMETERS)
{
	tbblue_force_disable_layer_sprites.v ^=1;
}

void menu_tbblue_layer_settings_ula(MENU_ITEM_PARAMETERS)
{
	tbblue_force_disable_layer_ula.v ^=1;
}

void menu_tbblue_layer_settings_tilemap(MENU_ITEM_PARAMETERS)
{
	tbblue_force_disable_layer_tilemap.v ^=1;
}

void menu_tbblue_layer_settings_layer_two(MENU_ITEM_PARAMETERS)
{
	tbblue_force_disable_layer_layer_two.v ^=1;
}

void menu_tbblue_layer_reveal_ula(MENU_ITEM_PARAMETERS)
{
	tbblue_reveal_layer_ula.v ^=1;
}

void menu_tbblue_layer_reveal_layer2(MENU_ITEM_PARAMETERS)
{
	tbblue_reveal_layer_layer2.v ^=1;
}

void menu_tbblue_layer_reveal_sprites(MENU_ITEM_PARAMETERS)
{
	tbblue_reveal_layer_sprites.v ^=1;
}



void menu_tsconf_layer_reveal_ula(MENU_ITEM_PARAMETERS)
{
	tsconf_reveal_layer_ula.v ^=1;
}

void menu_tsconf_layer_reveal_sprites_zero(MENU_ITEM_PARAMETERS)
{
	tsconf_reveal_layer_sprites_zero.v ^=1;
}

void menu_tsconf_layer_reveal_sprites_one(MENU_ITEM_PARAMETERS)
{
	tsconf_reveal_layer_sprites_one.v ^=1;
}


void menu_tsconf_layer_reveal_sprites_two(MENU_ITEM_PARAMETERS)
{
	tsconf_reveal_layer_sprites_two.v ^=1;
}


void menu_tsconf_layer_reveal_tiles_zero(MENU_ITEM_PARAMETERS)
{
	tsconf_reveal_layer_tiles_zero.v ^=1;
}

void menu_tsconf_layer_reveal_tiles_one(MENU_ITEM_PARAMETERS)
{
	tsconf_reveal_layer_tiles_one.v ^=1;
}

void menu_tsconf_layer_settings(MENU_ITEM_PARAMETERS)
{

	menu_espera_no_tecla();
	menu_reset_counters_tecla_repeticion();		


	int x=7;
	int y=1;

	int ancho=20;
	int alto=22;

	if (MACHINE_IS_TBBLUE) {
		alto=20;
		y=1;
	}
    //menu_dibuja_ventana(x,y,ancho,alto,"Video Layers");



	zxvision_window ventana;

	zxvision_new_window(&ventana,x,y,ancho,alto,
							ancho-1,alto-2,"Video Layers");
	zxvision_draw_window(&ventana);		





    //Cambiamos funcion overlay de texto de menu
    set_menu_overlay_function(menu_tsconf_layer_overlay);

	menu_tsconf_layer_overlay_window=&ventana; //Decimos que el overlay lo hace sobre la ventana que tenemos aqui	

    menu_item *array_menu_tsconf_layer_settings;
    menu_item item_seleccionado;
    int retorno_menu;						

    do {

		//Valido tanto para cuando multitarea es off y para que nada mas entrar aqui, se vea, sin tener que esperar el medio segundo 
		//que he definido en el overlay para que aparezca
		menu_tsconf_layer_overlay_mostrar_texto();

        int lin=1;

		if (MACHINE_IS_TSCONF) {

 			menu_add_item_menu_inicial_format(&array_menu_tsconf_layer_settings,MENU_OPCION_NORMAL,menu_tsconf_layer_settings_border,NULL,"%s",(tsconf_force_disable_layer_border.v ? "Disabled" : "Enabled "));
			menu_add_item_menu_tabulado(array_menu_tsconf_layer_settings,1,lin);
			lin+=3;			

			menu_add_item_menu_format(array_menu_tsconf_layer_settings,MENU_OPCION_NORMAL,menu_tsconf_layer_settings_ula,NULL,"%s",(tsconf_force_disable_layer_ula.v ? "Disabled" : "Enabled "));
			menu_add_item_menu_tabulado(array_menu_tsconf_layer_settings,1,lin);
			menu_add_item_menu_format(array_menu_tsconf_layer_settings,MENU_OPCION_NORMAL,menu_tsconf_layer_reveal_ula,NULL,"%s",(tsconf_reveal_layer_ula.v ? "Reveal" : "Normal"));
			menu_add_item_menu_tabulado(array_menu_tsconf_layer_settings,12,lin);		
			lin+=3;

			menu_add_item_menu_format(array_menu_tsconf_layer_settings,MENU_OPCION_NORMAL,menu_tsconf_layer_settings_sprites_zero,NULL,"%s",(tsconf_force_disable_layer_sprites_zero.v ? "Disabled" : "Enabled "));
			menu_add_item_menu_tabulado(array_menu_tsconf_layer_settings,1,lin);
			menu_add_item_menu_format(array_menu_tsconf_layer_settings,MENU_OPCION_NORMAL,menu_tsconf_layer_reveal_sprites_zero,NULL,"%s",(tsconf_reveal_layer_sprites_zero.v ? "Reveal" : "Normal"));
			menu_add_item_menu_tabulado(array_menu_tsconf_layer_settings,12,lin);				
			lin+=3;

			menu_add_item_menu_format(array_menu_tsconf_layer_settings,MENU_OPCION_NORMAL,menu_tsconf_layer_settings_tiles_zero,NULL,"%s",(tsconf_force_disable_layer_tiles_zero.v ? "Disabled" : "Enabled "));
			menu_add_item_menu_tabulado(array_menu_tsconf_layer_settings,1,lin);
			menu_add_item_menu_format(array_menu_tsconf_layer_settings,MENU_OPCION_NORMAL,menu_tsconf_layer_reveal_tiles_zero,NULL,"%s",(tsconf_reveal_layer_tiles_zero.v ? "Reveal" : "Normal"));
			menu_add_item_menu_tabulado(array_menu_tsconf_layer_settings,12,lin);						
			lin+=3;

			menu_add_item_menu_format(array_menu_tsconf_layer_settings,MENU_OPCION_NORMAL,menu_tsconf_layer_settings_sprites_one,NULL,"%s",(tsconf_force_disable_layer_sprites_one.v ? "Disabled" : "Enabled "));
			menu_add_item_menu_tabulado(array_menu_tsconf_layer_settings,1,lin);
			menu_add_item_menu_format(array_menu_tsconf_layer_settings,MENU_OPCION_NORMAL,menu_tsconf_layer_reveal_sprites_one,NULL,"%s",(tsconf_reveal_layer_sprites_one.v ? "Reveal" : "Normal"));
			menu_add_item_menu_tabulado(array_menu_tsconf_layer_settings,12,lin);					
			lin+=3;

			menu_add_item_menu_format(array_menu_tsconf_layer_settings,MENU_OPCION_NORMAL,menu_tsconf_layer_settings_tiles_one,NULL,"%s",(tsconf_force_disable_layer_tiles_one.v ? "Disabled" : "Enabled "));
			menu_add_item_menu_tabulado(array_menu_tsconf_layer_settings,1,lin);
			menu_add_item_menu_format(array_menu_tsconf_layer_settings,MENU_OPCION_NORMAL,menu_tsconf_layer_reveal_tiles_one,NULL,"%s",(tsconf_reveal_layer_tiles_one.v ? "Reveal" : "Normal"));
			menu_add_item_menu_tabulado(array_menu_tsconf_layer_settings,12,lin);				
			lin+=3;

			menu_add_item_menu_format(array_menu_tsconf_layer_settings,MENU_OPCION_NORMAL,menu_tsconf_layer_settings_sprites_two,NULL,"%s",(tsconf_force_disable_layer_sprites_two.v ? "Disabled" : "Enabled "));
			menu_add_item_menu_tabulado(array_menu_tsconf_layer_settings,1,lin);
			menu_add_item_menu_format(array_menu_tsconf_layer_settings,MENU_OPCION_NORMAL,menu_tsconf_layer_reveal_sprites_two,NULL,"%s",(tsconf_reveal_layer_sprites_two.v ? "Reveal" : "Normal"));
			menu_add_item_menu_tabulado(array_menu_tsconf_layer_settings,12,lin);					
			lin+=3;

		}

		if (MACHINE_IS_TBBLUE) {
 			menu_add_item_menu_inicial_format(&array_menu_tsconf_layer_settings,MENU_OPCION_NORMAL,menu_tbblue_layer_settings_ula,NULL,"%s",(tbblue_force_disable_layer_ula.v ? "Disabled" : "Enabled "));
			menu_add_item_menu_tabulado(array_menu_tsconf_layer_settings,1,lin);		
			lin+=3;			

 			menu_add_item_menu_format(array_menu_tsconf_layer_settings,MENU_OPCION_NORMAL,menu_tbblue_layer_settings_tilemap,NULL,"%s",(tbblue_force_disable_layer_tilemap.v ? "Disabled" : "Enabled "));
			menu_add_item_menu_tabulado(array_menu_tsconf_layer_settings,1,lin);			
			lin+=2;

 			menu_add_item_menu_format(array_menu_tsconf_layer_settings,MENU_OPCION_NORMAL,menu_tbblue_layer_reveal_ula,NULL,"%s",(tbblue_reveal_layer_ula.v ? "Reveal" : "Normal"));
			menu_add_item_menu_tabulado(array_menu_tsconf_layer_settings,12,lin);	

			lin+=3;					

			menu_add_item_menu_format(array_menu_tsconf_layer_settings,MENU_OPCION_NORMAL,menu_tbblue_layer_settings_sprites,NULL,"%s",(tbblue_force_disable_layer_sprites.v ? "Disabled" : "Enabled "));
			menu_add_item_menu_tabulado(array_menu_tsconf_layer_settings,1,lin);
 			menu_add_item_menu_format(array_menu_tsconf_layer_settings,MENU_OPCION_NORMAL,menu_tbblue_layer_reveal_sprites,NULL,"%s",(tbblue_reveal_layer_sprites.v ? "Reveal" : "Normal"));
			menu_add_item_menu_tabulado(array_menu_tsconf_layer_settings,12,lin);				
			lin+=3;

			menu_add_item_menu_format(array_menu_tsconf_layer_settings,MENU_OPCION_NORMAL,menu_tbblue_layer_settings_layer_two,NULL,"%s",(tbblue_force_disable_layer_layer_two.v ? "Disabled" : "Enabled "));
			menu_add_item_menu_tabulado(array_menu_tsconf_layer_settings,1,lin);
 			menu_add_item_menu_format(array_menu_tsconf_layer_settings,MENU_OPCION_NORMAL,menu_tbblue_layer_reveal_layer2,NULL,"%s",(tbblue_reveal_layer_layer2.v ? "Reveal" : "Normal"));
			menu_add_item_menu_tabulado(array_menu_tsconf_layer_settings,12,lin);				
			lin+=3;				
		}


				

        retorno_menu=menu_dibuja_menu(&tsconf_layer_settings_opcion_seleccionada,&item_seleccionado,array_menu_tsconf_layer_settings,"TSConf Layers" );

	//En caso de menus tabulados, es responsabilidad de este de borrar la ventana
        cls_menu_overlay();

				//Nombre de ventana solo aparece en el caso de stdout
                if ((item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu>=0) {
                        //llamamos por valor de funcion
                        if (item_seleccionado.menu_funcion!=NULL) {
                                //printf ("actuamos por funcion\n");
                                item_seleccionado.menu_funcion(item_seleccionado.valor_opcion);
				//En caso de menus tabulados, es responsabilidad de este de borrar la ventana
                                
                        }
                }

        } while ( (item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu!=MENU_RETORNO_ESC && !salir_todos_menus);

       //restauramos modo normal de texto de menu
       set_menu_overlay_function(normal_overlay_texto_menu);

	   cls_menu_overlay();

	//En caso de menus tabulados, es responsabilidad de este de liberar ventana
	zxvision_destroy_window(&ventana);			   


}







#define TOTAL_PALETTE_WINDOW_X 0
#define TOTAL_PALETTE_WINDOW_Y 0
#define TOTAL_PALETTE_WINDOW_ANCHO 32
#define TOTAL_PALETTE_WINDOW_ALTO 24
#define TOTAL_PALETTE_COLORS_PER_WINDOW 16



int menu_display_total_palette_current_palette=0;
int menu_display_total_palette_current_colour=0;

//Si se muestra paleta total o paleta mapeada
int menu_display_total_palette_show_mapped=0;

//Retorna colores totales de una paleta ya sea total o mapeada
int menu_display_total_palette_get_total_colors(void)
{
	int limite;

	if (menu_display_total_palette_show_mapped==0) {
		limite=total_palette_colours_array[menu_display_total_palette_current_palette].total_colores;
	}
	else {
		limite=menu_debug_sprites_total_colors_mapped_palette(menu_display_total_palette_current_palette);
	}

	return limite;
}

zxvision_window *menu_display_total_palette_draw_barras_window;

//Muestra lista de colores o barras de colores, para una paleta total, o para la paleta mapeada
int menu_display_total_palette_lista_colores(int linea,int si_barras)
{

	char dumpmemoria[33];

	int linea_color;
	int limite;


	limite=menu_display_total_palette_get_total_colors();

	int current_color;
	int indice_paleta;
	int indice_color_final_rgb;
	int color_final_rgb;

		for (linea_color=0;linea_color<TOTAL_PALETTE_COLORS_PER_WINDOW &&
				menu_display_total_palette_current_colour+linea_color<limite;
				linea_color++) {

					current_color=menu_display_total_palette_current_colour+linea_color;

					int digitos_hexa;
					int digitos_dec;

					digitos_dec=menu_debug_get_total_digits_dec(limite-1);



					if (menu_display_total_palette_show_mapped==0) {

						indice_paleta=total_palette_colours_array[menu_display_total_palette_current_palette].indice_inicial;
						indice_color_final_rgb=indice_paleta+current_color;
						color_final_rgb=spectrum_colortable_normal[indice_color_final_rgb];



						sprintf (dumpmemoria,"%*d: RGB %06XH",digitos_dec,current_color,color_final_rgb);
					}

					else {
						indice_paleta=menu_debug_sprites_return_index_palette(menu_display_total_palette_current_palette, current_color);
						indice_color_final_rgb=menu_debug_sprites_return_color_palette(menu_display_total_palette_current_palette,current_color);
						color_final_rgb=spectrum_colortable_normal[indice_color_final_rgb];
						digitos_hexa=menu_debug_get_total_digits_hexa((menu_debug_sprites_max_value_mapped_palette(menu_display_total_palette_current_palette))-1);

						int no_mostrar_indice=0;

						//Spectra ni speccy base no usan tabla de paleta
						if (menu_display_total_palette_current_palette==2 || menu_display_total_palette_current_palette==0) no_mostrar_indice=1;

						if (no_mostrar_indice) {
							sprintf (dumpmemoria,"%*d: RGB %06XH",digitos_dec,indice_paleta,color_final_rgb);
						}
						else {
							sprintf (dumpmemoria,"%*d: %0*XH RGB %06XH",digitos_dec,current_color,digitos_hexa,indice_paleta,color_final_rgb);
						}

					}



					int longitud_texto=strlen(dumpmemoria);

					int posicion_barra_color_x=TOTAL_PALETTE_WINDOW_X+longitud_texto+2;
					int posicion_barra_color_y=TOTAL_PALETTE_WINDOW_Y+3+linea_color;

					//dibujar la barra de color
					if (si_barras) {
						menu_dibuja_rectangulo_relleno(menu_display_total_palette_draw_barras_window,posicion_barra_color_x*menu_char_width,posicion_barra_color_y*8,
											menu_char_width*(TOTAL_PALETTE_WINDOW_ANCHO-longitud_texto-3),8,indice_color_final_rgb);
					}

			 		else {
						//menu_escribe_linea_opcion(linea++,-1,1,dumpmemoria);
						zxvision_print_string_defaults_fillspc(menu_display_total_palette_draw_barras_window,1,linea++,dumpmemoria);
					}
		}

	zxvision_draw_window_contents(menu_display_total_palette_draw_barras_window);
 

	return linea;
}




void menu_display_total_palette_draw_barras(void)
{

				menu_speech_tecla_pulsada=1; //Si no, envia continuamente todo ese texto a speech


				//Mostrar lista colores
				menu_display_total_palette_lista_colores(TOTAL_PALETTE_WINDOW_Y+3,0);

				//Esto tiene que estar despues de escribir la lista de colores, para que se refresque y se vea
				//Si estuviese antes, al mover el cursor hacia abajo dejándolo pulsado, el texto no se vería hasta que no se soltase la tecla
				normal_overlay_texto_menu();

				if (si_complete_video_driver()) {
					//Mostrar colores
					menu_display_total_palette_lista_colores(0,1);
				}
}

void menu_display_total_palette_cursor_arriba(void)
{
	if (menu_display_total_palette_current_colour>0) {
		menu_display_total_palette_current_colour--;
	}
}

void menu_display_total_palette_cursor_abajo(void)
{

	int limite=menu_display_total_palette_get_total_colors();

	if (menu_display_total_palette_current_colour<limite-1) {
		menu_display_total_palette_current_colour++;
	}

}

void menu_display_total_palette(MENU_ITEM_PARAMETERS)
{
	menu_espera_no_tecla();
	menu_reset_counters_tecla_repeticion();		

	zxvision_window ventana;

	zxvision_new_window(&ventana,TOTAL_PALETTE_WINDOW_X,TOTAL_PALETTE_WINDOW_Y,TOTAL_PALETTE_WINDOW_ANCHO,TOTAL_PALETTE_WINDOW_ALTO,
							TOTAL_PALETTE_WINDOW_ANCHO-1,TOTAL_PALETTE_WINDOW_ALTO-2,"Colour palettes");
	zxvision_draw_window(&ventana);

	z80_byte tecla;


	int salir=0;


	set_menu_overlay_function(menu_display_total_palette_draw_barras);
	menu_display_total_palette_draw_barras_window=&ventana; //Decimos que el overlay lo hace sobre la ventana que tenemos aqui

    do {

		//Borramos lista de colores con espacios por si hay estos de antes

        //Forzar a mostrar atajos
        z80_bit antes_menu_writing_inverse_color;
        antes_menu_writing_inverse_color.v=menu_writing_inverse_color.v;
        menu_writing_inverse_color.v=1;		
		
		int i;
		for (i=0;i<16;i++) zxvision_print_string_defaults_fillspc(&ventana,0,TOTAL_PALETTE_WINDOW_Y+3+i,"");

        menu_speech_tecla_pulsada=0; //Que envie a speech

		int linea=0;

		char textoshow[33];

		char nombre_paleta[33];

		if (menu_display_total_palette_show_mapped==0) {
			strcpy(nombre_paleta,total_palette_colours_array[menu_display_total_palette_current_palette].nombre_paleta);
		}
		else {
			menu_debug_sprites_get_palette_name(menu_display_total_palette_current_palette,nombre_paleta);
		}

		sprintf (textoshow,"Palette %d: %s",menu_display_total_palette_current_palette,nombre_paleta);
       	//menu_escribe_linea_opcion(linea++,-1,1,textoshow);
		zxvision_print_string_defaults_fillspc(&ventana,1,linea++,textoshow);

		if (menu_display_total_palette_show_mapped==0) {
			sprintf (textoshow,"%s",total_palette_colours_array[menu_display_total_palette_current_palette].descripcion_paleta);
		}
		else {
			sprintf (textoshow,"Total colours in array: %d",menu_display_total_palette_get_total_colors() );
		}
		//menu_escribe_linea_opcion(linea++,-1,1,textoshow);
		zxvision_print_string_defaults_fillspc(&ventana,1,linea++,textoshow);

   		//menu_escribe_linea_opcion(linea++,-1,1,"");
		zxvision_print_string_defaults_fillspc(&ventana,1,linea++,"");

		//linea=menu_display_total_palette_lista_colores(linea,0);
		linea +=16;


		//printf ("zone size: %x dir: %x\n",menu_display_memory_zone_size,menu_display_total_palette_direccion);

        //menu_escribe_linea_opcion(linea++,-1,1,"");
		zxvision_print_string_defaults_fillspc(&ventana,1,linea++,"");

		char buffer_linea[40];

		linea=TOTAL_PALETTE_WINDOW_Y+TOTAL_PALETTE_COLORS_PER_WINDOW+4;

															// 01234567890123456789012345678901
		sprintf (buffer_linea,"Move: Cursors,Q,A,PgUp,PgDn");

		//menu_escribe_linea_opcion(linea++,-1,1,buffer_linea);
		zxvision_print_string_defaults_fillspc(&ventana,1,linea++,buffer_linea);

		sprintf (buffer_linea,"[%c] ~~Mapped palette",(menu_display_total_palette_show_mapped ? 'X' : ' ') );
		//menu_escribe_linea_opcion(linea++,-1,1,buffer_linea);
		zxvision_print_string_defaults_fillspc(&ventana,1,linea++,buffer_linea);

        //Restaurar comportamiento atajos
        menu_writing_inverse_color.v=antes_menu_writing_inverse_color.v;

		zxvision_draw_window_contents(&ventana);
			
		tecla=zxvision_common_getkey_refresh();		

		int aux_pgdnup;
		int limite;

				switch (tecla) {

					case 11:
						//arriba
						menu_display_total_palette_cursor_arriba();

						//menu_display_total_palette_ventana();
						//menu_display_total_palette_direccion -=bytes_por_linea;
						//menu_display_total_palette_direccion=menu_display_total_palette_adjusta_en_negativo(menu_display_total_palette_direccion,bytes_por_linea);
					break;

					case 10:
						//abajo
						menu_display_total_palette_cursor_abajo();

						//menu_display_total_palette_ventana();


					break;

					case 24:
						//PgUp
						for (aux_pgdnup=0;aux_pgdnup<TOTAL_PALETTE_COLORS_PER_WINDOW;aux_pgdnup++) {
							menu_display_total_palette_cursor_arriba();
						}
						//menu_display_total_palette_ventana();

						//menu_display_total_palette_direccion -=bytes_por_ventana;
						//menu_display_total_palette_direccion=menu_display_total_palette_adjusta_en_negativo(menu_display_total_palette_direccion,bytes_por_ventana);
					break;

					case 25:
						//PgDn
						for (aux_pgdnup=0;aux_pgdnup<TOTAL_PALETTE_COLORS_PER_WINDOW;aux_pgdnup++) {
							menu_display_total_palette_cursor_abajo();
						}
						//menu_display_total_palette_ventana();
					break;

					case 'q':
						if (menu_display_total_palette_current_palette>0) {
							menu_display_total_palette_current_palette--;
							menu_display_total_palette_current_colour=0;
						}

						//menu_display_total_palette_ventana();
						//menu_display_total_palette_direccion=menu_display_total_palette_change_pointer(menu_display_total_palette_direccion);
						//menu_display_total_palette_ventana();
					break;

					case 'a':
						if (menu_display_total_palette_show_mapped==0) {
							limite=TOTAL_PALETAS_COLORES;
						}

						else {
							limite=MENU_TOTAL_MAPPED_PALETTES;
						}

						if (menu_display_total_palette_current_palette<limite-1) {
							menu_display_total_palette_current_palette++;
							menu_display_total_palette_current_colour=0;
						}


						//menu_display_total_palette_ventana();
						//menu_display_total_palette_direccion=menu_display_total_palette_change_pointer(menu_display_total_palette_direccion);
						//menu_display_total_palette_ventana();
					break;

					case 'm':
						menu_display_total_palette_show_mapped ^=1;
						menu_display_total_palette_current_palette=0;
						menu_display_total_palette_current_colour=0;
						//menu_display_total_palette_ventana();
					break;


					//Salir con ESC
					case 2:
						salir=1;
					break;
				}


        } while (salir==0);

				//restauramos modo normal de texto de menu
        set_menu_overlay_function(normal_overlay_texto_menu);


	cls_menu_overlay();
	zxvision_destroy_window(&ventana);


}

void menu_debug_disassemble_export(int p)
{

	char string_address[10];
	sprintf (string_address,"%XH",p);


    menu_ventana_scanf("Start?",string_address,10);

	menu_z80_moto_int inicio=parse_string_to_number(string_address);

	menu_ventana_scanf("End?",string_address,10);
	menu_z80_moto_int final=parse_string_to_number(string_address);

	if (final<inicio){
		menu_warn_message("End address must be higher or equal than start address");
		return;
	}

	char file_save[PATH_MAX];	
	int ret=menu_ask_file_to_save("Destination file","asm",file_save);

	if (!ret) {
		menu_warn_message("Export cancelled");
		return;
	}


	FILE *ptr_asmfile;
    ptr_asmfile=fopen(file_save,"wb");
    if (!ptr_asmfile) {
		debug_printf (VERBOSE_ERR,"Unable to open asm file");
		return;
    }
                  
 
	char dumpassembler[65];


	int longitud_opcode;

	//ponemos un final de un numero maximo de instrucciones
	//sera igual al tamaño de la zona de memoria
	int limite_instrucciones=menu_debug_memory_zone_size;

	int instrucciones=0;

	for (;inicio<=final && instrucciones<limite_instrucciones;instrucciones++) {

		menu_debug_dissassemble_una_inst_sino_hexa(dumpassembler,inicio,&longitud_opcode,0,0);


		inicio +=longitud_opcode;
		debug_printf (VERBOSE_DEBUG,"Exporting asm: %s",dumpassembler);

		//Agregar salto de linea
		int longitud_linea=strlen(dumpassembler);
		dumpassembler[longitud_linea++]='\n';
		dumpassembler[longitud_linea]=0;
		fwrite(&dumpassembler,1,longitud_linea,ptr_asmfile);
		//zxvision_print_string_defaults_fillspc(&ventana,1,linea,dumpassembler);
	}	

	fclose(ptr_asmfile);

	menu_generic_message_splash("Export disassembly","Ok process finished");

}

z80_bit menu_debug_disassemble_hexa_view={0};

void menu_debug_disassemble(MENU_ITEM_PARAMETERS)
{

	//printf ("Opening disassemble menu\n");
 	menu_espera_no_tecla();
	menu_reset_counters_tecla_repeticion();		

	zxvision_window ventana;

	int ancho_total=32-1;

	if (CPU_IS_MOTOROLA) ancho_total=64-1;

	zxvision_new_window(&ventana,0,1,32,20,
							ancho_total,20-2,"Disassemble");
	zxvision_draw_window(&ventana);			

    //Inicializar info de tamanyo zona
	menu_debug_set_memory_zone_attr();



	z80_byte tecla;

    
    menu_z80_moto_int direccion=menu_debug_disassemble_last_ptr;
		

	do {
		int linea=0;

		int lineas_disass=0;
		const int lineas_total=15;

		char dumpassembler[65];

		int longitud_opcode;
		int longitud_opcode_primera_linea;

		menu_z80_moto_int dir=direccion;

		for (;lineas_disass<lineas_total;lineas_disass++,linea++) {

			//Formato de texto en buffer:
			//0123456789012345678901234567890
			//DDDD AABBCCDD OPCODE-----------
			//DDDD: Direccion
			//AABBCCDD: Volcado hexa

			//Metemos 30 espacios
		


			//menu_debug_dissassemble_una_instruccion(dumpassembler,dir,&longitud_opcode);
			menu_debug_dissassemble_una_inst_sino_hexa(dumpassembler,dir,&longitud_opcode,menu_debug_disassemble_hexa_view.v,1);


			if (lineas_disass==0) longitud_opcode_primera_linea=longitud_opcode;

			dir +=longitud_opcode;
			zxvision_print_string_defaults_fillspc(&ventana,1,linea,dumpassembler);
		}


	//Forzar a mostrar atajos
	z80_bit antes_menu_writing_inverse_color;
	antes_menu_writing_inverse_color.v=menu_writing_inverse_color.v;
	menu_writing_inverse_color.v=1;



        zxvision_print_string_defaults_fillspc(&ventana,1,linea++,"");

        zxvision_print_string_defaults_fillspc(&ventana,1,linea++,"~~M: Ch. pointer ~~E: Export");
		zxvision_print_string_defaults_fillspc(&ventana,1,linea++,"~~T: Toggle hexa");

		zxvision_draw_window_contents(&ventana);


	//Restaurar comportamiento atajos
	menu_writing_inverse_color.v=antes_menu_writing_inverse_color.v;




		tecla=zxvision_common_getkey_refresh();				

                     
		int i;

        switch (tecla) {

			case 11:
				//arriba
				direccion=menu_debug_disassemble_subir(direccion);
			break;

			case 10:
				//abajo
				direccion +=longitud_opcode_primera_linea;
			break;

			//No llamamos a zxvision_handle_cursors_pgupdn para solo poder gestionar scroll ventana en horizontal,
			//el resto de teclas (cursores, pgup, dn etc) las gestionamos desde aqui de manera diferente

            //izquierda
            case 8:
				/*
				//Decir que se ha pulsado tecla para que no se relea
				menu_speech_tecla_pulsada=1;*/
				zxvision_handle_cursors_pgupdn(&ventana,tecla);
            break;

            //derecha
            case 9:
				/*
				//Decir que se ha pulsado tecla para que no se relea
				menu_speech_tecla_pulsada=1;*/
				zxvision_handle_cursors_pgupdn(&ventana,tecla);
			break;					

			case 24:
				//PgUp
				for (i=0;i<lineas_total;i++) direccion=menu_debug_disassemble_subir(direccion);
			break;

			case 25:
				//PgDn
				direccion=dir;
			break;

			case 'm':
				//Usamos misma funcion de menu_debug_hexdump_change_pointer
				direccion=menu_debug_hexdump_change_pointer(direccion);
				zxvision_draw_window(&ventana);
			break;

			case 'e':
				menu_debug_disassemble_export(direccion);
				zxvision_draw_window(&ventana);
			break;

			case 't':
				menu_debug_disassemble_hexa_view.v ^=1;
			break;

		}


	} while (tecla!=2); 


    cls_menu_overlay();
	zxvision_destroy_window(&ventana);		

 

}


void menu_debug_assemble(MENU_ITEM_PARAMETERS)
{

	//printf ("Opening disassemble menu\n");
 	menu_espera_no_tecla();
	menu_reset_counters_tecla_repeticion();		

	zxvision_window ventana;

	int ancho_total=32-1;

	if (CPU_IS_MOTOROLA) ancho_total=64-1;

	zxvision_new_window(&ventana,0,1,32,20,
							ancho_total,20-2,"Assemble");
	zxvision_draw_window(&ventana);			

    //Inicializar info de tamanyo zona
	menu_debug_set_memory_zone_attr();


    
    menu_z80_moto_int direccion=menu_debug_disassemble_last_ptr;

	menu_z80_moto_int direccion_ensamblado=direccion;

	int salir=0;
	int lineas_ensambladas=0;
		

	do {
		int linea=0;

		int lineas_disass=0;
		const int lineas_total=15;

		char dumpassembler[65];

		int longitud_opcode;
		int longitud_opcode_primera_linea;

		menu_z80_moto_int dir=direccion;

		for (;lineas_disass<lineas_total;lineas_disass++,linea++) {

			//Formato de texto en buffer:
			//0123456789012345678901234567890
			//DDDD AABBCCDD OPCODE-----------
			//DDDD: Direccion
			//AABBCCDD: Volcado hexa

			//Metemos 30 espacios
		


			//menu_debug_dissassemble_una_instruccion(dumpassembler,dir,&longitud_opcode);
			menu_debug_dissassemble_una_inst_sino_hexa(dumpassembler,dir,&longitud_opcode,menu_debug_disassemble_hexa_view.v,1);


			if (lineas_disass==0) longitud_opcode_primera_linea=longitud_opcode;

			dir +=longitud_opcode;
			zxvision_print_string_defaults_fillspc(&ventana,1,linea,dumpassembler);
		}


		zxvision_draw_window_contents(&ventana);



		char string_opcode[256];
		string_opcode[0]=0;


		char texto_titulo[256];
		sprintf (texto_titulo,"Assemble at %XH",direccion_ensamblado);

    	menu_ventana_scanf(texto_titulo,string_opcode,256);
		zxvision_draw_window(&ventana);

		if (string_opcode[0]==0) salir=1;
		else {


				z80_byte destino_ensamblado[256];


				int longitud_destino=assemble_opcode(direccion_ensamblado,string_opcode,destino_ensamblado);

				if (longitud_destino==0) {
					menu_error_message("Error. Invalid opcode");
					//escribir_socket_format(misocket,"Error. Invalid opcode: %s\n",texto);
					salir=1;
				}

				else {
					menu_debug_set_memory_zone_attr();
					unsigned int direccion_escribir=direccion_ensamblado;
					int i;
					for (i=0;i<longitud_destino;i++) {
						menu_debug_write_mapped_byte(direccion_escribir++,destino_ensamblado[i]);
					}

				}

				direccion_ensamblado+=longitud_destino;

				lineas_ensambladas++;

				//Desensamblar desde la siguiente instruccion si conviene
				if (lineas_ensambladas>5) {
					direccion +=longitud_opcode_primera_linea;
				}

		}
	
	} while (!salir);


		

    cls_menu_overlay();
	zxvision_destroy_window(&ventana);		

 

}



void menu_spectrum_core_reduced(MENU_ITEM_PARAMETERS)
{
	core_spectrum_uses_reduced.v ^=1;

	set_cpu_core_loop();

}



void menu_tbblue_deny_turbo_rom(MENU_ITEM_PARAMETERS)
{

	tbblue_deny_turbo_rom.v ^=1;
}

void menu_hardware_top_speed(MENU_ITEM_PARAMETERS)
{
	top_speed_timer.v ^=1;
}

void menu_turbo_mode(MENU_ITEM_PARAMETERS)
{
	if (cpu_turbo_speed==MAX_CPU_TURBO_SPEED) cpu_turbo_speed=1;
	else cpu_turbo_speed *=2;

	cpu_set_turbo_speed();
}

void menu_zxuno_deny_turbo_bios_boot(MENU_ITEM_PARAMETERS)
{
	zxuno_deny_turbo_bios_boot.v ^=1;
}

//menu cpu settings
void menu_cpu_settings(MENU_ITEM_PARAMETERS)
{
    menu_item *array_menu_cpu_settings;
    menu_item item_seleccionado;
    int retorno_menu;
    do {

		//hotkeys usadas: todc

		char buffer_velocidad[16];

		if (CPU_IS_Z80 && !MACHINE_IS_Z88) {
			int cpu_hz=get_cpu_frequency();
			int cpu_khz=cpu_hz/1000;

			//Obtener decimales
			int mhz_enteros=cpu_khz/1000;
			int decimal_mhz=cpu_khz-(mhz_enteros*1000);

								//01234567890123456789012345678901
								//           1234567890
								//Turbo: 16X 99.999 MHz
			sprintf(buffer_velocidad,"%d.%d MHz",mhz_enteros,decimal_mhz);
		}
		else {
			buffer_velocidad[0]=0;
		}

		menu_add_item_menu_inicial_format(&array_menu_cpu_settings,MENU_OPCION_NORMAL,menu_turbo_mode,NULL,"~~Turbo [%dX %s]",cpu_turbo_speed,buffer_velocidad);
		menu_add_item_menu_shortcut(array_menu_cpu_settings,'t');
		menu_add_item_menu_tooltip(array_menu_cpu_settings,"Changes only the Z80 speed");
		menu_add_item_menu_ayuda(array_menu_cpu_settings,"Changes only the Z80 speed. Do not modify FPS, interrupts or any other parameter. "
					"Some machines, like ZX-Uno or Chloe, change this setting");

		if (MACHINE_IS_ZXUNO) {
					menu_add_item_menu_format(array_menu_cpu_settings,MENU_OPCION_NORMAL,menu_zxuno_deny_turbo_bios_boot,NULL,"[%c] ~~Deny turbo on boot",
							(zxuno_deny_turbo_bios_boot.v ? 'X' : ' ') );
					menu_add_item_menu_shortcut(array_menu_cpu_settings,'d');
					menu_add_item_menu_tooltip(array_menu_cpu_settings,"Denies changing turbo mode when booting ZX-Uno and on bios");
					menu_add_item_menu_ayuda(array_menu_cpu_settings,"Denies changing turbo mode when booting ZX-Uno and on bios");
	  }

		if (MACHINE_IS_TBBLUE) {
					menu_add_item_menu_format(array_menu_cpu_settings,MENU_OPCION_NORMAL,menu_tbblue_deny_turbo_rom,NULL,"[%c] ~~Deny turbo on ROM",
							(tbblue_deny_turbo_rom.v ? 'X' : ' ') );
					menu_add_item_menu_shortcut(array_menu_cpu_settings,'d');
					menu_add_item_menu_tooltip(array_menu_cpu_settings,"Denies changing turbo mode on Next ROM. Useful on slow machines. Can make the boot process to fail");
					menu_add_item_menu_ayuda(array_menu_cpu_settings,"Denies changing turbo mode on Next ROM. Useful on slow machines. Can make the boot process to fail");
	  }	  

		if (!MACHINE_IS_Z88) {
			menu_add_item_menu_format(array_menu_cpu_settings,MENU_OPCION_NORMAL,menu_hardware_top_speed,NULL,"[%c] T~~op Speed",(top_speed_timer.v ? 'X' : ' ') );
			menu_add_item_menu_shortcut(array_menu_cpu_settings,'o');
			menu_add_item_menu_tooltip(array_menu_cpu_settings,"Runs at maximum speed, when menu closed. Not available on Z88");
			menu_add_item_menu_ayuda(array_menu_cpu_settings,"Runs at maximum speed, using 100% of CPU of host machine, when menu closed. "
						"The display is refreshed 1 time per second. This mode is also entered when loading a real tape and "
						"accelerate loaders setting is enabled. Not available on Z88");

		}	  

		if (MACHINE_IS_SPECTRUM) {
			menu_add_item_menu_format(array_menu_cpu_settings,MENU_OPCION_NORMAL,menu_spectrum_core_reduced,NULL,"Spectrum ~~core [%s]",
			(core_spectrum_uses_reduced.v ? "Reduced" : "Normal") );
			menu_add_item_menu_shortcut(array_menu_cpu_settings,'c');
			menu_add_item_menu_tooltip(array_menu_cpu_settings,"Switches between the normal Spectrum core or the reduced core");
			menu_add_item_menu_ayuda(array_menu_cpu_settings,"When using the Spectrum reduced core, the following features are NOT available or are NOT properly emulated:\n"
				"Debug t-states, Char detection, +3 Disk, Save to tape, Divide, Divmmc, Multiface, RZX, Raster interrupts, ZX-Uno DMA, TBBlue DMA, Datagear DMA, TBBlue Copper, Audio DAC, Stereo AY, Video out to file, Last core frame statistics");
		}



                menu_add_item_menu(array_menu_cpu_settings,"",MENU_OPCION_SEPARADOR,NULL,NULL);

                menu_add_ESC_item(array_menu_cpu_settings);

                retorno_menu=menu_dibuja_menu(&cpu_settings_opcion_seleccionada,&item_seleccionado,array_menu_cpu_settings,"CPU Settings" );

                
                if ((item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu>=0) {
                        //llamamos por valor de funcion
                        if (item_seleccionado.menu_funcion!=NULL) {
                                //printf ("actuamos por funcion\n");
                                item_seleccionado.menu_funcion(item_seleccionado.valor_opcion);
                                
                        }
                }

        } while ( (item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu!=MENU_RETORNO_ESC && !salir_todos_menus);



}



void menu_display_snow_effect(MENU_ITEM_PARAMETERS)
{
	snow_effect_enabled.v ^=1;
}


void menu_display_inves_ula_bright_error(MENU_ITEM_PARAMETERS)
{
	inves_ula_bright_error.v ^=1;
}


void menu_display_slow_adjust(MENU_ITEM_PARAMETERS)
{
	video_zx8081_lnctr_adjust.v ^=1;
}



void menu_display_estabilizador_imagen(MENU_ITEM_PARAMETERS)
{
	video_zx8081_estabilizador_imagen.v ^=1;
}

void menu_display_interlace(MENU_ITEM_PARAMETERS)
{
	if (video_interlaced_mode.v) disable_interlace();
	else enable_interlace();
}


void menu_display_interlace_scanlines(MENU_ITEM_PARAMETERS)
{
	if (video_interlaced_scanlines.v) disable_scanlines();
	else enable_scanlines();
}

void menu_display_gigascreen(MENU_ITEM_PARAMETERS)
{
        if (gigascreen_enabled.v) disable_gigascreen();
        else enable_gigascreen();
}

void menu_display_chroma81(MENU_ITEM_PARAMETERS)
{
        if (chroma81.v) disable_chroma81();
        else enable_chroma81();
}

void menu_display_ulaplus(MENU_ITEM_PARAMETERS)
{
        if (ulaplus_presente.v) disable_ulaplus();
        else enable_ulaplus();
}


void menu_display_autodetect_chroma81(MENU_ITEM_PARAMETERS)
{
	autodetect_chroma81.v ^=1;
}


void menu_display_spectra(MENU_ITEM_PARAMETERS)
{
	if (spectra_enabled.v) spectra_disable();
	else spectra_enable();
}

void menu_display_snow_effect_margin(MENU_ITEM_PARAMETERS)
{
	snow_effect_min_value++;
	if (snow_effect_min_value==8) snow_effect_min_value=1;
}

void menu_display_timex_video(MENU_ITEM_PARAMETERS)
{
	if (timex_video_emulation.v) disable_timex_video();
	else enable_timex_video();
}

void menu_display_minimo_vsync(MENU_ITEM_PARAMETERS)
{

        menu_hardware_advanced_input_value(100,999,"Minimum vsync lenght",&minimo_duracion_vsync);
}

void menu_display_timex_video_512192(MENU_ITEM_PARAMETERS)
{

	timex_mode_512192_real.v ^=1;
}

void menu_display_cpc_force_mode(MENU_ITEM_PARAMETERS)
{
	if (cpc_forzar_modo_video.v==0) {
		cpc_forzar_modo_video.v=1;
		cpc_forzar_modo_video_modo=0;
	}
	else {
		cpc_forzar_modo_video_modo++;
		if (cpc_forzar_modo_video_modo==4) {
			cpc_forzar_modo_video_modo=0;
			cpc_forzar_modo_video.v=0;
		}
	}
}

void menu_display_refresca_sin_colores(MENU_ITEM_PARAMETERS)
{
	scr_refresca_sin_colores.v ^=1;
	modificado_border.v=1;
}


void menu_display_timex_force_line_512192(MENU_ITEM_PARAMETERS)
{
	if (timex_ugly_hack_last_hires==0) timex_ugly_hack_last_hires=198;

	        char string_num[4];

        sprintf (string_num,"%d",timex_ugly_hack_last_hires);

        menu_ventana_scanf("Scanline",string_num,4);

        timex_ugly_hack_last_hires=parse_string_to_number(string_num);
}

void menu_display_timex_ugly_hack(MENU_ITEM_PARAMETERS)
{
	timex_ugly_hack_enabled ^=1;
}

void menu_spritechip(MENU_ITEM_PARAMETERS)
{
	if (spritechip_enabled.v) spritechip_disable();
	else spritechip_enable();
}


void menu_display_emulate_fast_zx8081(MENU_ITEM_PARAMETERS)
{
	video_fast_mode_emulation.v ^=1;
	modificado_border.v=1;
}



void menu_display_emulate_zx8081display_spec(MENU_ITEM_PARAMETERS)
{
	if (simulate_screen_zx8081.v==1) simulate_screen_zx8081.v=0;
	else {
		simulate_screen_zx8081.v=1;
		umbral_simulate_screen_zx8081=4;
	}
	modificado_border.v=1;
}


void menu_display_osd_word_kb_length(MENU_ITEM_PARAMETERS)
{

	char string_length[4];

        sprintf (string_length,"%d",adventure_keyboard_key_length);

        menu_ventana_scanf("Length? (10-100)",string_length,4);

        int valor=parse_string_to_number(string_length);
	if (valor<10 || valor>100) {
		debug_printf (VERBOSE_ERR,"Invalid value");
	}

	else {
		adventure_keyboard_key_length=valor;
	}

}


void menu_display_osd_word_kb_finalspc(MENU_ITEM_PARAMETERS)
{
	adventure_keyboard_send_final_spc ^=1;
}


void menu_display_emulate_zx8081_thres(MENU_ITEM_PARAMETERS)
{

        char string_thres[3];

        sprintf (string_thres,"%d",umbral_simulate_screen_zx8081);

        menu_ventana_scanf("Pixel Threshold",string_thres,3);

	umbral_simulate_screen_zx8081=parse_string_to_number(string_thres);
	if (umbral_simulate_screen_zx8081<1 || umbral_simulate_screen_zx8081>16) umbral_simulate_screen_zx8081=4;

}


int menu_display_settings_disp_zx8081_spectrum(void)
{

	//esto solo en spectrum y si el driver no es curses y si no hay rainbow
	if (!strcmp(scr_driver_name,"curses")) return 0;
	if (rainbow_enabled.v==1) return 0;

	return !menu_cond_zx8081();
}


void menu_display_arttext(MENU_ITEM_PARAMETERS)
{
	texto_artistico.v ^=1;
}



#ifdef COMPILE_AA
void menu_display_slowaa(MENU_ITEM_PARAMETERS)
{
	scraa_fast ^=1;
}
#else
void menu_display_slowaa(MENU_ITEM_PARAMETERS){}
#endif



void menu_display_zx8081_wrx(MENU_ITEM_PARAMETERS)
{
	if (wrx_present.v) {
		disable_wrx();
	}

	else {
		enable_wrx();
	}
	//wrx_present.v ^=1;
}





void menu_display_x_offset(MENU_ITEM_PARAMETERS)
{
	offset_zx8081_t_coordx +=8;
        if (offset_zx8081_t_coordx>=30*8) offset_zx8081_t_coordx=-30*8;
}


int menu_display_emulate_zx8081_cond(void)
{
	return simulate_screen_zx8081.v;
}


void menu_display_autodetect_rainbow(MENU_ITEM_PARAMETERS)
{
	autodetect_rainbow.v ^=1;
}

void menu_display_autodetect_wrx(MENU_ITEM_PARAMETERS)
{
        autodetect_wrx.v ^=1;
}

int menu_display_aa_cond(void)
{
        if (!strcmp(scr_driver_name,"aa")) return 1;

        else return 0;
}


void menu_display_tsconf_vdac(MENU_ITEM_PARAMETERS)
{
	tsconf_vdac_with_pwm.v ^=1;

	menu_interface_rgb_inverse_common();
}

void menu_display_tsconf_pal_depth(MENU_ITEM_PARAMETERS)
{
	tsconf_palette_depth--;
	if (tsconf_palette_depth<2) tsconf_palette_depth=5;

	menu_interface_rgb_inverse_common();

}

void menu_display_rainbow(MENU_ITEM_PARAMETERS)
{
	if (rainbow_enabled.v==0) enable_rainbow();
	else disable_rainbow();


}

void menu_vofile_insert(MENU_ITEM_PARAMETERS)
{

        if (vofile_inserted.v==0) {
                init_vofile();
                //Si todo ha ido bien
                if (vofile_inserted.v) {
                        menu_generic_message_format("File information","%s\n%s\n\n%s",
												last_message_helper_aofile_vofile_file_format,last_message_helper_aofile_vofile_bytes_minute_video,last_message_helper_aofile_vofile_util);
                }

        }

        else if (vofile_inserted.v==1) {
                close_vofile();
        }

}


int menu_vofile_cond(void)
{
        if (vofilename!=NULL) return 1;
        else return 0;
}

void menu_vofile(MENU_ITEM_PARAMETERS)
{

        vofile_inserted.v=0;


        char *filtros[2];

        filtros[0]="rwv";
        filtros[1]=0;


        if (menu_filesel("Select Video File",filtros,vofilename_file)==1) {

                 //Ver si archivo existe y preguntar
                struct stat buf_stat;

                if (stat(vofilename_file, &buf_stat)==0) {

                        if (menu_confirm_yesno_texto("File exists","Overwrite?")==0) {
                                vofilename=NULL;
                                return;
                        }

                }



                vofilename=vofilename_file;
        }

        else {
                vofilename=NULL;
        }


}

void menu_vofile_fps(MENU_ITEM_PARAMETERS)
{
	if (vofile_fps==1) {
		vofile_fps=50;
		return;
	}

        if (vofile_fps==2) {
                vofile_fps=1;
                return;
        }


        if (vofile_fps==5) {
                vofile_fps=2;
                return;
        }

        if (vofile_fps==10) {
                vofile_fps=5;
                return;
        }

        if (vofile_fps==25) {
                vofile_fps=10;
                return;
        }


        if (vofile_fps==50) {
                vofile_fps=25;
                return;
        }

}

int menu_display_curses_cond(void)
{
	if (!strcmp(scr_driver_name,"curses")) return 1;

	else return 0;
}


int menu_display_cursesstdout_cond(void)
{
	if (menu_display_curses_cond() ) return 1;
	if (menu_cond_stdout() ) return 1;

	return 0;
}



int menu_display_cursesstdoutsimpletext_cond(void)
{
	if (menu_display_cursesstdout_cond() ) return 1;
	if (menu_cond_simpletext() ) return 1;

	return 0;
}



int menu_display_arttext_cond(void)
{

	if (!menu_display_cursesstdout_cond()) return 0;

	//en zx80 y 81 no hay umbral, no tiene sentido. ahora si. hay rainbow de zx8081
	//if (machine_type>=20 && machine_type<=21) return 0;

        return texto_artistico.v;
}

int menu_cond_stdout_simpletext(void)
{
	if (menu_cond_stdout() || menu_cond_simpletext() ) return 1;
	return 0;
}


//En curses y stdout solo se permite para zx8081
int menu_cond_realvideo_curses_stdout_zx8081(void)
{
	if (menu_cond_stdout() || menu_cond_curses() ) {
		if (MACHINE_IS_SPECTRUM ) return 0;
	}

	return 1;
}


void menu_display_stdout_simpletext_automatic_redraw(MENU_ITEM_PARAMETERS)
{
	stdout_simpletext_automatic_redraw.v ^=1;
}



void menu_display_send_ansi(MENU_ITEM_PARAMETERS)
{
	screen_text_accept_ansi ^=1;
}

void menu_display_arttext_thres(MENU_ITEM_PARAMETERS)
{

        char string_thres[3];

        sprintf (string_thres,"%d",umbral_arttext);

        menu_ventana_scanf("Pixel Threshold",string_thres,3);

        umbral_arttext=parse_string_to_number(string_thres);
        if (umbral_arttext<1 || umbral_arttext>16) umbral_arttext=4;

}


void menu_display_text_brightness(MENU_ITEM_PARAMETERS)
{

        char string_bri[4];

        sprintf (string_bri,"%d",screen_text_brightness);

        menu_ventana_scanf("Brightness? (0-100)",string_bri,4);

	int valor=parse_string_to_number(string_bri);
	if (valor<0 || valor>100) debug_printf (VERBOSE_ERR,"Invalid brightness value %d",valor);

	else screen_text_brightness=valor;

}






void menu_display_stdout_simpletext_fps(MENU_ITEM_PARAMETERS)
{
	    char string_fps[3];

        sprintf (string_fps,"%d",50/scrstdout_simpletext_refresh_factor);

        menu_ventana_scanf("FPS? (1-50)",string_fps,3);

        int valor=parse_string_to_number(string_fps);
		scr_set_fps_stdout_simpletext(valor);

}


void menu_textdrivers_settings(MENU_ITEM_PARAMETERS)
{
        menu_item *array_menu_textdrivers_settings;
        menu_item item_seleccionado;
        int retorno_menu;
        do {

		char buffer_string[50];


		//Como no sabemos cual sera el item inicial, metemos este sin asignar, que se sobreescribe en el siguiente menu_add_item_menu
		//menu_add_item_menu_inicial(&array_menu_textdrivers_settings,"---Text Driver Settings--",MENU_OPCION_UNASSIGNED,NULL,NULL);
		menu_add_item_menu_inicial(&array_menu_textdrivers_settings,"",MENU_OPCION_UNASSIGNED,NULL,NULL);


                //para stdout y simpletext
                if (menu_cond_stdout_simpletext() ) {
                        menu_add_item_menu_format(array_menu_textdrivers_settings,MENU_OPCION_NORMAL,menu_display_stdout_simpletext_automatic_redraw,NULL,"Stdout automatic redraw: %s", (stdout_simpletext_automatic_redraw.v==1 ? "On" : "Off"));
                        menu_add_item_menu_tooltip(array_menu_textdrivers_settings,"It enables automatic display redraw");
                        menu_add_item_menu_ayuda(array_menu_textdrivers_settings,"It enables automatic display redraw");


                        menu_add_item_menu_format(array_menu_textdrivers_settings,MENU_OPCION_NORMAL,menu_display_send_ansi,NULL,"Send ANSI Control Sequence: %s",(screen_text_accept_ansi==1 ? "On" : "Off"));

						if (stdout_simpletext_automatic_redraw.v) {
							menu_add_item_menu_format(array_menu_textdrivers_settings,MENU_OPCION_NORMAL,menu_display_stdout_simpletext_fps,NULL,"Redraw fps: %d", 50/scrstdout_simpletext_refresh_factor);
						}

                }

		if (menu_display_cursesstdout_cond() ) {
                        //solo en caso de curses o stdout
                        menu_add_item_menu_format(array_menu_textdrivers_settings,MENU_OPCION_NORMAL,menu_display_arttext,menu_display_cursesstdout_cond,"Text artistic emulation: %s", (texto_artistico.v==1 ? "On" : "Off"));
                        menu_add_item_menu_tooltip(array_menu_textdrivers_settings,"Write different artistic characters for unknown 4x4 rectangles, "
                                        "on stdout and curses drivers");

                        menu_add_item_menu_ayuda(array_menu_textdrivers_settings,"Write different artistic characters for unknown 4x4 rectangles, "
                                        "on curses, stdout and simpletext drivers. "
                                        "If disabled, unknown characters are written with ?");


                        menu_add_item_menu_format(array_menu_textdrivers_settings,MENU_OPCION_NORMAL,menu_display_arttext_thres,menu_display_arttext_cond,"Pixel threshold: %d",umbral_arttext);
                        menu_add_item_menu_tooltip(array_menu_textdrivers_settings,"Pixel Threshold to decide which artistic character write in a 4x4 rectangle, "
                                        "on curses, stdout and simpletext drivers with text artistic emulation enabled");
                        menu_add_item_menu_ayuda(array_menu_textdrivers_settings,"Pixel Threshold to decide which artistic character write in a 4x4 rectangle, "
                                        "on curses, stdout and simpletext drivers with text artistic emulation enabled");

                        if (rainbow_enabled.v) {
                                menu_add_item_menu_format(array_menu_textdrivers_settings,MENU_OPCION_NORMAL,menu_display_text_brightness,NULL,"Text brightness: %d",screen_text_brightness);
                                menu_add_item_menu_tooltip(array_menu_textdrivers_settings,"Text brightness used on some machines and text drivers, like tsconf");
                                menu_add_item_menu_ayuda(array_menu_textdrivers_settings,"Text brightness used on some machines and text drivers, like tsconf");
                        }

                }






if (menu_display_aa_cond() ) {

#ifdef COMPILE_AA
                        sprintf (buffer_string,"Slow AAlib emulation: %s", (scraa_fast==0 ? "On" : "Off"));
#else
                        sprintf (buffer_string,"Slow AAlib emulation: Off");
#endif
                        menu_add_item_menu(array_menu_textdrivers_settings,buffer_string,MENU_OPCION_NORMAL,menu_display_slowaa,menu_display_aa_cond);

                        menu_add_item_menu_tooltip(array_menu_textdrivers_settings,"Enable slow aalib emulation; slow is a little better");
                        menu_add_item_menu_ayuda(array_menu_textdrivers_settings,"Enable slow aalib emulation; slow is a little better");

                }



                menu_add_item_menu(array_menu_textdrivers_settings,"",MENU_OPCION_SEPARADOR,NULL,NULL);
                //menu_add_item_menu(array_menu_textdrivers_settings,"ESC Back",MENU_OPCION_NORMAL|MENU_OPCION_ESC,NULL,NULL);
                menu_add_ESC_item(array_menu_textdrivers_settings);

                retorno_menu=menu_dibuja_menu(&textdrivers_settings_opcion_seleccionada,&item_seleccionado,array_menu_textdrivers_settings,"Text Driver Settings" );

                

                if ((item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu>=0) {
                        //llamamos por valor de funcion
                        if (item_seleccionado.menu_funcion!=NULL) {
                                //printf ("actuamos por funcion\n");
                                item_seleccionado.menu_funcion(item_seleccionado.valor_opcion);
                                
                        }
                }

        } while ( (item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu!=MENU_RETORNO_ESC && !salir_todos_menus);

}



void menu_display_cpc_double_vsync(MENU_ITEM_PARAMETERS)
{
	cpc_send_double_vsync.v ^=1;
}



//menu display settings
void menu_settings_display(MENU_ITEM_PARAMETERS)
{

	menu_item *array_menu_settings_display;
	menu_item item_seleccionado;
	int retorno_menu;
	do {

		//char string_aux[50],string_aux2[50],emulate_zx8081_disp[50],string_arttext[50],string_aaslow[50],emulate_zx8081_thres[50],string_arttext_threshold[50];
		//char buffer_string[50];

		//hotkeys usadas: ricglwmptez


                char string_vofile_shown[10];
                menu_tape_settings_trunc_name(vofilename,string_vofile_shown,10);


	                menu_add_item_menu_inicial_format(&array_menu_settings_display,MENU_OPCION_NORMAL,menu_vofile,NULL,"Video out to file: %s",string_vofile_shown);
        	        menu_add_item_menu_tooltip(array_menu_settings_display,"Saves the video output to a file");
                	menu_add_item_menu_ayuda(array_menu_settings_display,"The generated file have raw format. You can see the file parameters "
					"on the console enabling verbose debug level to 2 minimum.\n"
					"A watermark is added to the final video, as you may see when you activate it\n"
					"Note: Gigascreen, Interlaced effects or menu windows are not saved to file."

					);

			if (menu_vofile_cond() ) {
				menu_add_item_menu_format(array_menu_settings_display,MENU_OPCION_NORMAL,menu_vofile_fps,menu_vofile_cond,"[%d] FPS Video file",50/vofile_fps);
	        	menu_add_item_menu_format(array_menu_settings_display,MENU_OPCION_NORMAL,menu_vofile_insert,menu_vofile_cond,"[%c] Video file enabled",(vofile_inserted.v ? 'X' : ' ' ));
			}

			else {
                	  menu_add_item_menu(array_menu_settings_display,"",MENU_OPCION_SEPARADOR,NULL,NULL);
			}



                if (!MACHINE_IS_Z88) {


                        menu_add_item_menu_format(array_menu_settings_display,MENU_OPCION_NORMAL,menu_display_autodetect_rainbow,NULL,"[%c] Autodetect Real Video",(autodetect_rainbow.v==1 ? 'X' : ' '));
                        menu_add_item_menu_tooltip(array_menu_settings_display,"Autodetect the need to enable Real Video");
                        menu_add_item_menu_ayuda(array_menu_settings_display,"This option detects whenever is needed to enable Real Video. "
                                        "On Spectrum, it detects the reading of idle bus or repeated border changes. "
                                        "On ZX80/81, it detects the I register on a non-normal value when executing video display. "
					"On all machines, it also detects when loading a real tape. "
                                        );
                }




			menu_add_item_menu_format(array_menu_settings_display,MENU_OPCION_NORMAL,menu_display_rainbow,menu_display_rainbow_cond,"[%c] ~~Real Video",(rainbow_enabled.v==1 ? 'X' : ' '));
			menu_add_item_menu_shortcut(array_menu_settings_display,'r');

			menu_add_item_menu_tooltip(array_menu_settings_display,"Enable Real Video. Enabling it makes display as a real machine");
			menu_add_item_menu_ayuda(array_menu_settings_display,"Real Video makes display works as in the real machine. It uses a bit more CPU than disabling it.\n\n"
					"On Spectrum, display is drawn every scanline. "
					"It enables hi-res colour (rainbow) on the screen and on the border, Gigascreen, Interlaced, ULAplus, Spectra, Timex Video, snow effect, idle bus reading and some other advanced features. "
					"Also enables all the Inves effects.\n"
					"Disabling it, the screen is drawn once per frame (1/50) and the previous effects "
					"are not supported.\n\n"
					"On ZX80/ZX81, enables hi-res display and loading/saving stripes on the screen, and the screen is drawn every scanline.\n"
					"By disabling it, the screen is drawn once per frame, no hi-res display, and only text mode is supported.\n\n"
					"On Z88, display is drawn the same way as disabling it; it is only used when enabling Video out to file.\n\n"
					"Real Video can be enabled on all the video drivers, but on curses, stdout and simpletext (in Spectrum and Z88 machines), the display drawn is the same "
					"as on non-Real Video, but you can have idle bus support on these drivers. "
					"Curses, stdout and simpletext drivers on ZX80/81 machines do have Real Video display."
					);

                if (MACHINE_IS_TSCONF) {
                        menu_add_item_menu_format(array_menu_settings_display,MENU_OPCION_NORMAL,menu_display_tsconf_vdac,NULL,"[%c] TSConf VDAC PWM",
                        (tsconf_vdac_with_pwm.v ? 'X' : ' ')     );

                        menu_add_item_menu_tooltip(array_menu_settings_display,"Enables full vdac colour palette or PWM style");
			menu_add_item_menu_ayuda(array_menu_settings_display,"Full vdac colour palette gives you different colour levels for every 5 bit colour component.\n"
					"With PWM mode it gives you 5 bit values different from 0..23, but from 24 to 31 are all set to value 255");


			menu_add_item_menu_format(array_menu_settings_display,MENU_OPCION_NORMAL,menu_display_tsconf_pal_depth,NULL,
					 "[%d] TSConf palette depth",tsconf_palette_depth);




                }


		if (MACHINE_IS_CPC) {
				if (cpc_forzar_modo_video.v==0)
					menu_add_item_menu_format(array_menu_settings_display,MENU_OPCION_NORMAL,menu_display_cpc_force_mode,NULL,"[ ] Force Video Mode");
				else
					menu_add_item_menu_format(array_menu_settings_display,MENU_OPCION_NORMAL,menu_display_cpc_force_mode,NULL,"[%d] Force Video Mode",
						cpc_forzar_modo_video_modo);

				menu_add_item_menu_format(array_menu_settings_display,MENU_OPCION_NORMAL,menu_display_cpc_double_vsync,NULL,"[%c] Double Vsync",(cpc_send_double_vsync.v==1 ? 'X' : ' ') );
				menu_add_item_menu_tooltip(array_menu_settings_display,"Workaround to avoid hang on some games");
				menu_add_item_menu_ayuda(array_menu_settings_display,"Workaround to avoid hang on some games");
		}


		if (!MACHINE_IS_Z88) {


			if (menu_cond_realvideo() ) {
				menu_add_item_menu_format(array_menu_settings_display,MENU_OPCION_NORMAL,menu_display_interlace,menu_cond_realvideo,"[%c] ~~Interlaced mode", (video_interlaced_mode.v==1 ? 'X' : ' '));
				menu_add_item_menu_shortcut(array_menu_settings_display,'i');
				menu_add_item_menu_tooltip(array_menu_settings_display,"Enable interlaced mode");
				menu_add_item_menu_ayuda(array_menu_settings_display,"Interlaced mode draws the screen like the machine on a real TV: "
					"Every odd frame, odd lines on TV are drawn; every even frame, even lines on TV are drawn. It can be used "
					"to emulate twice the vertical resolution of the machine (384) or simulate different colours. "
					"This effect is only emulated with vertical zoom multiple of two: 2,4,6... etc");


				if (video_interlaced_mode.v) {
					menu_add_item_menu_format(array_menu_settings_display,MENU_OPCION_NORMAL,menu_display_interlace_scanlines,NULL,"[%c] S~~canlines", (video_interlaced_scanlines.v==1 ? 'X' : ' '));
					menu_add_item_menu_shortcut(array_menu_settings_display,'c');
					menu_add_item_menu_tooltip(array_menu_settings_display,"Enable scanlines on interlaced mode");
					menu_add_item_menu_ayuda(array_menu_settings_display,"Scanlines draws odd lines a bit darker than even lines");
				}


				if (!MACHINE_IS_TBBLUE) {
					menu_add_item_menu_format(array_menu_settings_display,MENU_OPCION_NORMAL,menu_display_gigascreen,NULL,"[%c] ~~Gigascreen",(gigascreen_enabled.v==1 ? 'X' : ' '));
					menu_add_item_menu_shortcut(array_menu_settings_display,'g');
					menu_add_item_menu_tooltip(array_menu_settings_display,"Enable gigascreen colours");
					menu_add_item_menu_ayuda(array_menu_settings_display,"Gigascreen enables more than 15 colours by combining pixels "
							"of even and odd frames. The total number of different colours is 102");
				}



				if (MACHINE_IS_SPECTRUM && !MACHINE_IS_ZXEVO && !MACHINE_IS_TBBLUE)  {

					menu_add_item_menu_format(array_menu_settings_display,MENU_OPCION_NORMAL,menu_display_snow_effect,NULL,"[%c] Snow effect support", (snow_effect_enabled.v==1 ? 'X' : ' '));
					menu_add_item_menu_tooltip(array_menu_settings_display,"Enable snow effect on Spectrum");
					menu_add_item_menu_ayuda(array_menu_settings_display,"Snow effect is a bug on some Spectrum models "
						"(models except +2A and +3) that draws corrupted pixels when I register is pointed to "
						"slow RAM.");
						// Even on 48k models it resets the machine after some seconds drawing corrupted pixels");

					if (snow_effect_enabled.v==1) {
						menu_add_item_menu_format(array_menu_settings_display,MENU_OPCION_NORMAL,menu_display_snow_effect_margin,NULL,"[%d] Snow effect threshold",snow_effect_min_value);
					}
				}


				if (MACHINE_IS_INVES) {
					menu_add_item_menu_format(array_menu_settings_display,MENU_OPCION_NORMAL,menu_display_inves_ula_bright_error,NULL,"[%c] Inves bright error",(inves_ula_bright_error.v ? 'X' : ' '));
					menu_add_item_menu_tooltip(array_menu_settings_display,"Emulate Inves oddity when black colour and change from bright 0 to bright 1");
					menu_add_item_menu_ayuda(array_menu_settings_display,"Emulate Inves oddity when black colour and change from bright 0 to bright 1. Seems it only happens with RF or RGB connection");

				}



			}
		}

		//para stdout

		/*
#ifdef COMPILE_STDOUT
		if (menu_cond_stdout() ) {
			menu_add_item_menu_format(array_menu_settings_display,MENU_OPCION_NORMAL,menu_display_stdout_simpletext_automatic_redraw,NULL,"Stdout automatic redraw: %s", (stdout_simpletext_automatic_redraw.v==1 ? "On" : "Off"));
			menu_add_item_menu_tooltip(array_menu_settings_display,"It enables automatic display redraw");
			menu_add_item_menu_ayuda(array_menu_settings_display,"It enables automatic display redraw");


			menu_add_item_menu_format(array_menu_settings_display,MENU_OPCION_NORMAL,menu_display_send_ansi,NULL,"Send ANSI Control Sequence: %s",(screen_text_accept_ansi==1 ? "On" : "Off"));

		}

#endif

		*/


		if (menu_cond_zx8081_realvideo()) {

		//z80_bit video_zx8081_estabilizador_imagen;

			menu_add_item_menu_format(array_menu_settings_display,MENU_OPCION_NORMAL,menu_display_estabilizador_imagen,menu_cond_zx8081_realvideo,"[%c] Horizontal stabilization", (video_zx8081_estabilizador_imagen.v==1 ? 'X' : ' '));
                        menu_add_item_menu_tooltip(array_menu_settings_display,"Horizontal image stabilization");
                        menu_add_item_menu_ayuda(array_menu_settings_display,"Horizontal image stabilization. Usually enabled.");


			menu_add_item_menu_format(array_menu_settings_display,MENU_OPCION_NORMAL,menu_display_slow_adjust,menu_cond_zx8081_realvideo,"[%c] ~~LNCTR video adjust", (video_zx8081_lnctr_adjust.v==1 ? 'X' : ' '));
			//l repetida con load screen, pero como esa es de spectrum, no coinciden
			menu_add_item_menu_shortcut(array_menu_settings_display,'l');
			menu_add_item_menu_tooltip(array_menu_settings_display,"LNCTR video adjust");
			menu_add_item_menu_ayuda(array_menu_settings_display,"LNCTR video adjust change sprite offset when drawing video images. "
				"If you see your hi-res image is not displayed well, try changing it");




        	        menu_add_item_menu_format(array_menu_settings_display,MENU_OPCION_NORMAL,menu_display_x_offset,menu_cond_zx8081_realvideo,"[%d] Video x_offset",offset_zx8081_t_coordx);
			menu_add_item_menu_tooltip(array_menu_settings_display,"Video horizontal image offset");
			menu_add_item_menu_ayuda(array_menu_settings_display,"Video horizontal image offset, usually you don't need to change this");


			menu_add_item_menu_format(array_menu_settings_display,MENU_OPCION_NORMAL,menu_display_minimo_vsync,menu_cond_zx8081_realvideo,"[%d] Video min. vsync lenght",minimo_duracion_vsync);
			menu_add_item_menu_tooltip(array_menu_settings_display,"Video minimum vsync lenght in t-states");
			menu_add_item_menu_ayuda(array_menu_settings_display,"Video minimum vsync lenght in t-states");


                        menu_add_item_menu_format(array_menu_settings_display,MENU_OPCION_NORMAL,menu_display_autodetect_wrx,NULL,"[%c] Autodetect WRX",(autodetect_wrx.v==1 ? 'X' : ' '));
                        menu_add_item_menu_tooltip(array_menu_settings_display,"Autodetect the need to enable WRX mode on ZX80/81");
                        menu_add_item_menu_ayuda(array_menu_settings_display,"This option detects whenever is needed to enable WRX. "
                                                "On ZX80/81, it detects the I register on a non-normal value when executing video display. "
						"In some cases, chr$128 and udg modes are detected incorrectly as WRX");


	                menu_add_item_menu_format(array_menu_settings_display,MENU_OPCION_NORMAL,menu_display_zx8081_wrx,menu_cond_zx8081_realvideo,"[%c] ~~WRX", (wrx_present.v ? 'X' : ' '));
			menu_add_item_menu_shortcut(array_menu_settings_display,'w');
			menu_add_item_menu_tooltip(array_menu_settings_display,"Enables WRX hi-res mode");
			menu_add_item_menu_ayuda(array_menu_settings_display,"Enables WRX hi-res mode");



		}

		else {

			if (menu_cond_zx8081() ) {
				menu_add_item_menu_format(array_menu_settings_display,MENU_OPCION_NORMAL,menu_display_emulate_fast_zx8081,menu_cond_zx8081_no_realvideo,"[%c] ZX80/81 detect fast mode", (video_fast_mode_emulation.v==1 ? 'X' : ' '));
				menu_add_item_menu_tooltip(array_menu_settings_display,"Detect fast mode and simulate it, on non-realvideo mode");
				menu_add_item_menu_ayuda(array_menu_settings_display,"Detect fast mode and simulate it, on non-realvideo mode");
			}

		}

		if (MACHINE_IS_ZX8081) {

			menu_add_item_menu_format(array_menu_settings_display,MENU_OPCION_NORMAL,menu_display_autodetect_chroma81,NULL,"[%c] Autodetect Chroma81",(autodetect_chroma81.v ? 'X' : ' '));
			menu_add_item_menu_tooltip(array_menu_settings_display,"Autodetect Chroma81");
			menu_add_item_menu_ayuda(array_menu_settings_display,"Detects when Chroma81 video mode is needed and enable it");


			menu_add_item_menu_format(array_menu_settings_display,MENU_OPCION_NORMAL,menu_display_chroma81,NULL,"[%c] Chro~~ma81 support",(chroma81.v ? 'X' : ' '));
			menu_add_item_menu_shortcut(array_menu_settings_display,'m');
			menu_add_item_menu_tooltip(array_menu_settings_display,"Enables Chroma81 colour video mode");
			menu_add_item_menu_ayuda(array_menu_settings_display,"Enables Chroma81 colour video mode");

		}


		if (MACHINE_IS_SPECTRUM && !MACHINE_IS_TBBLUE) {
			menu_add_item_menu_format(array_menu_settings_display,MENU_OPCION_NORMAL,menu_display_ulaplus,NULL,"[%c] ULA~~plus support",(ulaplus_presente.v ? 'X' : ' '));
			menu_add_item_menu_shortcut(array_menu_settings_display,'p');
			menu_add_item_menu_tooltip(array_menu_settings_display,"Enables ULAplus support");
			menu_add_item_menu_ayuda(array_menu_settings_display,"The following ULAplus modes are supported:\n"
						"Mode 1: Standard 256x192 64 colours\n"
						"Mode 3: Linear mode 128x96, 16 colours per pixel (radastan mode)\n"
						"Mode 5: Linear mode 256x96, 16 colours per pixel (ZEsarUX mode 0)\n"
						"Mode 7: Linear mode 128x192, 16 colours per pixel (ZEsarUX mode 1)\n"
						"Mode 9: Linear mode 256x192, 16 colours per pixel (ZEsarUX mode 2)\n"
			);
		}

		if (MACHINE_IS_SPECTRUM) {

			menu_add_item_menu_format(array_menu_settings_display,MENU_OPCION_NORMAL,menu_display_timex_video,NULL,"[%c] ~~Timex video support",(timex_video_emulation.v ? 'X' : ' '));
                        menu_add_item_menu_shortcut(array_menu_settings_display,'t');
                        menu_add_item_menu_tooltip(array_menu_settings_display,"Enables Timex Video modes");
                        menu_add_item_menu_ayuda(array_menu_settings_display,"The following Timex Video modes are emulated:\n"
						"Mode 0: Video data at address 16384 and 8x8 color attributes at address 22528 (like on ordinary Spectrum)\n"
						"Mode 1: Video data at address 24576 and 8x8 color attributes at address 30720\n"
						"Mode 2: Multicolor mode: video data at address 16384 and 8x1 color attributes at address 24576\n"
						"Mode 6: Hi-res mode 512x192, monochrome.");

			if (timex_video_emulation.v && !MACHINE_IS_TBBLUE) {
				menu_add_item_menu_format(array_menu_settings_display,MENU_OPCION_NORMAL,menu_display_timex_video_512192,NULL,"[%c] Timex Real 512x192",(timex_mode_512192_real.v ? 'X' : ' '));
				menu_add_item_menu_tooltip(array_menu_settings_display,"Selects between real 512x192 or scaled 256x192");
				menu_add_item_menu_ayuda(array_menu_settings_display,"Real 512x192 does not support scanline effects (it draws the display at once). "
							"If not enabled real, it draws scaled 256x192 but does support scanline effects");



				if (timex_mode_512192_real.v==0) {

					menu_add_item_menu_format(array_menu_settings_display,MENU_OPCION_NORMAL,menu_display_timex_ugly_hack,NULL,"[%c] Ugly hack",(timex_ugly_hack_enabled ? 'X' : ' ') );
					menu_add_item_menu_tooltip(array_menu_settings_display,"EXPERIMENTAL feature");
					menu_add_item_menu_ayuda(array_menu_settings_display,"EXPERIMENTAL feature");

					if (timex_ugly_hack_enabled) {
					menu_add_item_menu_format(array_menu_settings_display,MENU_OPCION_NORMAL,menu_display_timex_force_line_512192,NULL,"[%d] Force 512x192 at",timex_ugly_hack_last_hires);
					menu_add_item_menu_tooltip(array_menu_settings_display,"EXPERIMENTAL feature");
					menu_add_item_menu_ayuda(array_menu_settings_display,"EXPERIMENTAL feature");
					}
				}

			}



			if (!MACHINE_IS_ZXEVO) {

				menu_add_item_menu_format(array_menu_settings_display,MENU_OPCION_NORMAL,menu_display_spectra,NULL,"[%c] Sp~~ectra support",(spectra_enabled.v ? 'X' : ' '));
				menu_add_item_menu_shortcut(array_menu_settings_display,'e');
				menu_add_item_menu_tooltip(array_menu_settings_display,"Enables Spectra video modes");
				menu_add_item_menu_ayuda(array_menu_settings_display,"Enables Spectra video modes. All video modes are fully emulated");


				menu_add_item_menu_format(array_menu_settings_display,MENU_OPCION_NORMAL,menu_spritechip,NULL,"[%c] ~~ZGX Sprite Chip",(spritechip_enabled.v ? 'X' : ' ') );
				menu_add_item_menu_shortcut(array_menu_settings_display,'z');
				menu_add_item_menu_tooltip(array_menu_settings_display,"Enables ZGX Sprite Chip");
				menu_add_item_menu_ayuda(array_menu_settings_display,"Enables ZGX Sprite Chip");
			}


		}




		if (MACHINE_IS_SPECTRUM && rainbow_enabled.v==0) {
	                menu_add_item_menu(array_menu_settings_display,"",MENU_OPCION_SEPARADOR,NULL,NULL);


			menu_add_item_menu_format(array_menu_settings_display,MENU_OPCION_NORMAL,menu_display_emulate_zx8081display_spec,menu_display_settings_disp_zx8081_spectrum,"[%c] ZX80/81 Display on Speccy", (simulate_screen_zx8081.v==1 ? 'X' : ' '));
			menu_add_item_menu_tooltip(array_menu_settings_display,"Simulates the resolution of ZX80/81 on the Spectrum");
			menu_add_item_menu_ayuda(array_menu_settings_display,"It makes the resolution of display on Spectrum like a ZX80/81, with no colour. "
					"This mode is not supported with real video enabled");


			if (menu_display_emulate_zx8081_cond() ){
			menu_add_item_menu_format(array_menu_settings_display,MENU_OPCION_NORMAL,menu_display_emulate_zx8081_thres,menu_display_emulate_zx8081_cond,"[%d] Pixel threshold",umbral_simulate_screen_zx8081);
			menu_add_item_menu_tooltip(array_menu_settings_display,"Pixel Threshold to draw black or white in a 4x4 rectangle, "
					   "when ZX80/81 Display on Speccy enabled");
			menu_add_item_menu_ayuda(array_menu_settings_display,"Pixel Threshold to draw black or white in a 4x4 rectangle, "
					   "when ZX80/81 Display on Speccy enabled");
			}


			menu_add_item_menu_format(array_menu_settings_display,MENU_OPCION_NORMAL,menu_display_refresca_sin_colores,NULL,"[%c] Colours enabled",(scr_refresca_sin_colores.v==0 ? 'X' : ' '));
			menu_add_item_menu_tooltip(array_menu_settings_display,"Disables colours for Spectrum display");
			menu_add_item_menu_ayuda(array_menu_settings_display,"Disables colours for Spectrum display");



		}



		if (MACHINE_IS_SPECTRUM || MACHINE_IS_ZX8081 || MACHINE_IS_CPC) {
			menu_add_item_menu_format(array_menu_settings_display,MENU_OPCION_NORMAL,menu_display_osd_word_kb_length,NULL,"[%d] OSD Adventure KB length",adventure_keyboard_key_length);

			menu_add_item_menu_tooltip(array_menu_settings_display,"Define the duration for every key press on the Adventure Text OSD Keyboard");
			menu_add_item_menu_ayuda(array_menu_settings_display,"Define the duration for every key press on the Adventure Text OSD Keyboard, in 1/50 seconds (default 50)");

			menu_add_item_menu_format(array_menu_settings_display,MENU_OPCION_NORMAL,menu_display_osd_word_kb_finalspc,NULL,"[%c] OSD Adv. final space",
				(adventure_keyboard_send_final_spc ? 'X' : ' '));
					

			menu_add_item_menu_tooltip(array_menu_settings_display,"Sends a space after every word on the Adventure Text OSD Keyboard");
			menu_add_item_menu_ayuda(array_menu_settings_display,"Sends a space after every word on the Adventure Text OSD Keyboard");
			

		}


		if (menu_display_cursesstdoutsimpletext_cond() || menu_display_aa_cond() ) {
			menu_add_item_menu_format(array_menu_settings_display,MENU_OPCION_NORMAL,menu_textdrivers_settings,NULL,"Text driver settings");
		}

		


                menu_add_item_menu(array_menu_settings_display,"",MENU_OPCION_SEPARADOR,NULL,NULL);
                //menu_add_item_menu(array_menu_settings_display,"ESC Back",MENU_OPCION_NORMAL|MENU_OPCION_ESC,NULL,NULL);
		menu_add_ESC_item(array_menu_settings_display);

                retorno_menu=menu_dibuja_menu(&settings_display_opcion_seleccionada,&item_seleccionado,array_menu_settings_display,"Display Settings" );

                

		//NOTA: no llamar por numero de opcion dado que hay opciones que ocultamos (relacionadas con real video)

		if ((item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu>=0) {

			//llamamos por valor de funcion
        	        if (item_seleccionado.menu_funcion!=NULL) {
                	        //printf ("actuamos por funcion\n");
	                        item_seleccionado.menu_funcion(item_seleccionado.valor_opcion);
				
        	        }
		}

        } while ( (item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu!=MENU_RETORNO_ESC && !salir_todos_menus);




}




void menu_tbblue_machine_id(MENU_ITEM_PARAMETERS)
{

	menu_warn_message("Changing the machine id may show the Spectrum Next boot logo, which is NOT allowed. "
		"Please read the License file: https://gitlab.com/thesmog358/tbblue/blob/master/LICENSE.md");

        menu_item *array_menu_tbblue_hardware_id;
        menu_item item_seleccionado;
        int retorno_menu;

		menu_add_item_menu_inicial(&array_menu_tbblue_hardware_id,"",MENU_OPCION_UNASSIGNED,NULL,NULL);

                char buffer_texto[40];

                int i;
				int salir=0;
                for (i=0;i<=255 && !salir;i++) {

					z80_byte machine_id=tbblue_machine_id_list[i].id;
					if (machine_id==255) salir=1;
					else {

                  		sprintf (buffer_texto,"%3d %s",machine_id,tbblue_machine_id_list[i].nombre);

                        menu_add_item_menu_format(array_menu_tbblue_hardware_id,MENU_OPCION_NORMAL,NULL,NULL,buffer_texto);

						//Decir que no es custom 
						menu_add_item_menu_valor_opcion(array_menu_tbblue_hardware_id,0);
					}

				}

				menu_add_item_menu(array_menu_tbblue_hardware_id,"",MENU_OPCION_SEPARADOR,NULL,NULL);

				menu_add_item_menu_format(array_menu_tbblue_hardware_id,MENU_OPCION_NORMAL,NULL,NULL,"Custom");
				//Decir que es custom 
				menu_add_item_menu_valor_opcion(array_menu_tbblue_hardware_id,1);				

                menu_add_item_menu(array_menu_tbblue_hardware_id,"",MENU_OPCION_SEPARADOR,NULL,NULL);
                //menu_add_item_menu(array_menu_tbblue_hardware_id,"ESC Back",MENU_OPCION_NORMAL|MENU_OPCION_ESC,NULL,NULL);
                menu_add_ESC_item(array_menu_tbblue_hardware_id);

                retorno_menu=menu_dibuja_menu(&menu_tbblue_hardware_id_opcion_seleccionada,&item_seleccionado,array_menu_tbblue_hardware_id,"TBBlue machine id" );

                


				if ((item_seleccionado.tipo_opcion&MENU_OPCION_ESC)==0 && retorno_menu>=0) {

					//Si se pulsa Enter
					//Detectar si es la opcion de custom
					if (item_seleccionado.valor_opcion) {
        				char string_valor[4];
						sprintf (string_valor,"%d",tbblue_machine_id);

		                menu_ventana_scanf("ID?",string_valor,4);

        				tbblue_machine_id=parse_string_to_number(string_valor);

					}

					else {
						tbblue_machine_id=tbblue_machine_id_list[menu_tbblue_hardware_id_opcion_seleccionada].id;
					}
											

												
                }

}