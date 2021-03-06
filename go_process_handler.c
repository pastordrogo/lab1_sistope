#include "process_handler.h"
#include <math.h>


int main(int  argc, char ** argv){
	int iflag = 0;//Flag nombre de archivo de entrada
	int nflag = 0;//Flag numero procesos de comparardor
	int cflag = 0;//Flag cantidad de caracteres en una linea del archivo
	int pflag = 0;//Flag cadena a buscar
	int dflag = 0;//Flag mostrar resultados por pantalla
	char * input_file = NULL;
	int numero_procesos_comparador = 0;
	int cantidad_caracteres_en_linea = 0;
	char * cadena_a_buscar = NULL;
	int c;
	opterr=0;
	while((c = getopt(argc,argv,"i:n:c:p:d")) != -1){
		switch(c){
			case 'i':
				iflag = 1;
				input_file = malloc(strlen(optarg)+1);
				strcpy(input_file,optarg);
				break;
			case 'n':
				nflag = 1;
				sscanf(optarg, "%d", &numero_procesos_comparador);
				//numero_procesos_comparador = atoi(optarg);
				break;
			case 'c':
				cflag = 1;
				sscanf(optarg, "%d", &cantidad_caracteres_en_linea);
				//cantidad_caracteres_en_linea =atoi(optarg);
				break;
			case 'p':
				pflag = 1;
				cadena_a_buscar = malloc(strlen(optarg)+1);
				strcpy(cadena_a_buscar,optarg);
				break;
			case 'd':
				dflag = 1;
				break;
			case '?':
				if(optopt == 'i'){
					fprintf(stderr, "Opcion -%c requiere un argumento.\n", optopt);
				}else if(optopt == 'n'){
					fprintf(stderr, "Opcion -%c requiere un argumento.\n", optopt);
				}else if(optopt == 'c'){
					fprintf(stderr, "Opcion -%c requiere un argumento.\n", optopt);
				}else if(optopt == 'p'){
					fprintf(stderr, "Opcion -%c requiere un argumento.\n", optopt);
				}else if(isprint(optopt)){
					fprintf(stderr, "Opcion desconocida '-%c'.\n", optopt);
				}else{
					fprintf(stderr, "Opcion con caracter desconocido '\\x%x'.\n", optopt);
				}
				return 1;
			default:
				abort();
		}
	}
	// Comprobar que el largo de la cadena a buscar es menor o igual al largo
	// de cada linea
	if(strlen(cadena_a_buscar) > cantidad_caracteres_en_linea) {
		printf("El largo de la cadena que se quiere buscar es mayor a la cantidad de caracteres que hay en cada linea, por favor pruebe con otra cadena\n");
		return 1;
	}
	/*
		Prueba de paso de argumentos por línea de comandos
	*/
	/*printf("iflag = %d, nflag = %d, cflag = %d, pflag = %d, dflag = %d\n", iflag, nflag, cflag, pflag, dflag);
	printf("Archivo de entrada: %s\nNumero de procesos ha crear: %d\nCantidad de caracteres por linea: %d\nCadena ha comparar: %s\nImprime por pantalla: %s\n", 
		input_file,
		numero_procesos_comparador,
		cantidad_caracteres_en_linea,
		cadena_a_buscar,
		dflag ? "Si":"No"
		);
	*/
	/*
		Hasta acá 
	*/
	/*
		Procesamiento
	*/
	FILE* archivo = fopen(input_file, "r");
	if(archivo == NULL) {
		printf("El archivo %s no existe, ejecute el programa con un archivo existente\n", input_file);
		return 1;
	}
	int caracteres_reales_en_linea = calcularCaracteresRealesEnLinea(archivo);
	if (caracteres_reales_en_linea != cantidad_caracteres_en_linea){
		printf("La cantidad de caracteres por línea a leer es distinta número real de caracteres que contiene el archivo de entrada \"%s\"\n",input_file);
		return 1;
	}
	char ch; int i;
	pid_t* procesos_hijos;
	int lineas_totales, lineas_por_proceso, lineas_restantes, posicion_cursor = 0;
	lineas_totales = calcularLineas(archivo, cantidad_caracteres_en_linea);
	lineas_por_proceso = (int)ceil((float)lineas_totales / numero_procesos_comparador);
	lineas_restantes = lineas_totales;
	procesos_hijos = (pid_t*)calloc(numero_procesos_comparador, sizeof(int));

	for (int i = 0; i < numero_procesos_comparador; ++i)
	{
		int lineas_proceso = lineas_por_proceso;
		if(lineas_restantes - lineas_proceso < 0) {
			lineas_proceso = lineas_restantes;
		}

		procesos_hijos[i] = createProcess(i+1, input_file, posicion_cursor, lineas_proceso, cadena_a_buscar);

		lineas_restantes -= lineas_proceso;
		if(lineas_restantes == 0) {
			break;
		}

		// Obtener la siguiente posicion del cursor en el archivo.
		int lineas_aux = 0;
		while((ch = (char)getc(archivo)) != EOF) {
			if(ch == '\n') {
				lineas_aux++;
			}
			if(lineas_aux == lineas_proceso) {
				break;
			}
		}
		posicion_cursor = ftell(archivo);
	}

	/*
		Escribir resultados en un archivo.
	*/

	// Esperar que terminen todos los procesos creados.
	for (int i = 0; i < numero_procesos_comparador; ++i)
	{
		waitpid(procesos_hijos[i], NULL, 0);
	}

	int caracteres_por_proceso = (lineas_por_proceso) * (cantidad_caracteres_en_linea + 3 + 1);
	char* buffer = (char*)malloc(caracteres_por_proceso + 1);
	char* nombre_archivo_completo = getRcName(cadena_a_buscar);
	FILE* resultados_completos = fopen(nombre_archivo_completo, "w");
	for (int i = 1; i <= numero_procesos_comparador; ++i)
	{
		char* nombre_archivo_parcial = getRpName(cadena_a_buscar, i);
		FILE* resultado_parcial = fopen(nombre_archivo_parcial, "r");
		if(resultado_parcial == NULL) {
			continue;
		}

		fread(buffer, caracteres_por_proceso + 1, 1, resultado_parcial);
		fwrite(buffer, strlen(buffer), 1, resultados_completos);
		if(dflag == 1) {
			printf("%s", buffer);
		}

		fclose(resultado_parcial);
		free(nombre_archivo_parcial);
	}
	free(buffer);
	fclose(resultados_completos);
	free(nombre_archivo_completo);
	

	free(procesos_hijos);
	free(input_file);
	free(cadena_a_buscar);
	return 0;
}

