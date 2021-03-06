#include <jni.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tds.h"
#include "Parametros.h"
#include "Errores.h"
#include "Standar.h"
#include "Lexico.h"
#include "Parser.h"
#include "Conjuntos.h"
#include "Generador.h"

tds tabla;

struct claseCod *claseAct = NULL;
struct lsCod *lsCodActual = NULL;
int numActual = 0;





void clearScanner();
//Primera pasada funciones 
int programaP1(struct nodoArchivo *archivo, int conjunto[]);
int cuerpoP1(struct clase *clase, int conjunto[]);
int tipoDP1(struct atributo *atributo, int conjunto[]);

//Creo que ya no se usa
void copiarValor(struct atributo *dest, struct atributo *org);

//Segunda pasada funciones
int programa(struct nodoArchivo *, int toksig[]);
int cuerpo(struct nodoArchivo* miArchivo, struct clase *clase, int toksig[]);
int tipo(struct nodoArchivo *miArchivo, struct clase *clase, int toksig[]);
int metodo(struct nodoArchivo* miArchivo, struct clase *clase, int toksig[]);
int parametro(struct nodoArchivo *miArchivo, struct clase *clase, struct metodo*, int toksig[]);
int bloque(struct nodoArchivo* miArchivo, struct clase *clase, struct metodo *metodo, int toksig[]);
int instruccion(struct nodoArchivo* miArchivo, struct clase *clase, struct metodo *metodo, int toksig[]);
int instruccion_romper(int toksig[]);
int instruccion_si(struct nodoArchivo* miArchivo, struct clase *clase, struct metodo *metodo, int toksig[]);
int asignacionf(struct nodoArchivo* miArchivo, struct clase *clase, struct metodo *metodo, int toksig[]);
int expresion(struct nodoArchivo* miArchivo, struct clase *clase, struct metodo *metodo, int toksig[]);
int funcion_cadena(struct nodoArchivo* miArchivo, struct clase *clase, struct metodo *metodo, int toksig[]);
int instruccion_mientras(struct nodoArchivo* miArchivo, struct clase *clase, struct metodo *metodo, int toksig[]);
int instruccion_para(struct nodoArchivo* miArchivo, struct clase *clase, struct metodo *metodo, int toksig[]);
int instruccion_es(struct nodoArchivo* miArchivo, struct clase *clase, struct metodo *metodo, int toksig[]);
int expresion_numerica(struct nodoArchivo* miArchivo, struct clase *clase, struct metodo *metodo, int toksig[]);
int expresion_conjuncion(struct nodoArchivo* miArchivo, struct clase *clase, struct metodo *metodo, int toksig[]);
int expresion_relacional(struct nodoArchivo* miArchivo, struct clase *clase, struct metodo *metodo, int toksig[]);
int expresion_aritmetrica(struct nodoArchivo* miArchivo, struct clase *clase, struct metodo *metodo, int toksig[]);
int termino(struct nodoArchivo* miArchivo, struct clase *clase, struct metodo *metodo, int toksig[]);
int factor(struct nodoArchivo* miArchivo, struct clase *clase, struct metodo *metodo, int toksig[]);
int valor_cadena(struct nodoArchivo* miArchivo, struct clase *clase, struct metodo *metodo, int toksig[]);
int valor_caracter(struct nodoArchivo* miArchivo, struct clase *clase, struct metodo *metodo, int toksig[]);
int funcion_arreglo(struct nodoArchivo* miArchivo, struct clase *clase, struct metodo *metodo, int toksig[]);
int funcion_archivo(struct nodoArchivo* miArchivo, struct clase *clase, struct metodo *metodo, int toksig[]);
int llamada_metodo(struct nodoArchivo* miArchivo, struct clase *clase, struct metodo *metodo, int toksig[]);
int funcion_num_numcad(struct nodoArchivo* miArchivo, struct clase *clase, struct metodo *metodo, int toksig[]);
int instruccion_probar(struct nodoArchivo* miArchivo, struct clase *clase, struct metodo *metodo, int toksig[]);
int instruccion_hacer(struct nodoArchivo* miArchivo, struct clase *clase, struct metodo *metodo, int toksig[]);
int instruccion_paraCada(struct nodoArchivo* miArchivo, struct clase *clase, struct metodo *metodo, int toksig[]);

JNIEXPORT jobjectArray JNICALL Java_armus_lib_parser_Parser_run
(JNIEnv *env, jobject obj, jobjectArray jsLsFile) {

    //Cantidad de archivos mandados
    int cant = (*env)->GetArrayLength(env, jsLsFile);
    char **lsfiles; // Lista de archivos CONVERTIDAS a c
    void **jstrings;
    //reservado espacio para la conversion 
    lsfiles = (char **) malloc(sizeof (const char *) * cant);
    jstrings = (char **) malloc(sizeof (jstring *) * cant);
    int i;
    //Conversion 
    for (i = 0; i < cant; i++) {
        jstring string = (jstring) ((*env)->GetObjectArrayElement(env, jsLsFile, i));
        const char *file = (*env)->GetStringUTFChars(env, string, 0);
        lsfiles[i] = file;
        jstrings[i] = &string;
    }

    tabla.izq = NULL;
    tabla.dch = NULL;
    tabla.valor = NULL;
    inicializa_conjuntos();

  //printf("---------------------------Pasada 1 -----------------------------------\n");
    if (pasada1(lsfiles, cant) != -1) {
      //printf("---------------------------Pasada 2 -----------------------------------\n");
        if (pasada2(lsfiles, cant)) {
            //printf("Sin errores\n");
            return NULL; // Sin errores
        }
    }
    codigo.clase.hash = 0;
    
    
    //printf("Con %d errores\n", primerError);
    jobjectArray errores;

    errores = (*env)->NewObjectArray(env, primerError,
            (*env)->FindClass(env, "java/lang/String"), NULL);
    fp = fopen("log.txt", "r");

    if(primerError == 0){
        mostrar(codigo, lsfiles[0]);
    }
    int j;
    for (j = 0; j < primerError; j++) {
        getLine(linea, MAX_LINEA);
        (*env)->SetObjectArrayElement(env, errores, j, (*env)->NewStringUTF(env, linea));
    }
    //Relase Java things
    for (i = 0; i < cant; i++) {
        (*env)->ReleaseStringUTFChars(env, *((jstring *) jstrings[i]), lsfiles[i]);
    }

    return errores;
}

int pasada1(char **lsfiles, int cant) {
    int i;

    FILE * f;
    f = fopen("log.txt", "w");
    fprintf(f, "");
    fclose(f);

    clearScanner();

    //Hacerlo con cada archivo
    for (i = 0; i < cant; i++) {

        FILE * f;
        /*f = fopen("log.txt", "a");
        f//printf(f, "%s\n", lsfiles[i]);
        fclose(f);*/

        fp = fopen(lsfiles[i], "r");
        archivoActual = lsfiles[i];
        if (fp == NULL) {
            return -1; // si no se puede abrir el archivo
        }


        ch = ' ';
        fin_de_archivo = 0;
        offset = -1;
        ll = 0;
        LineaActual = 0;
        PosicionLineaActual = 0;
        struct nodoArchivo *miArchivo;
        instarArchivoTDS(lsfiles[i], &tabla, &miArchivo);

        obtoken();
        if (-1 == programaP1(miArchivo, set_arranque)) {
            fclose(fp);
            fp = NULL;
            return -1;
        };

        fclose(fp);
        fp = NULL;
    }
    return 1;
}

int pasada2(char **lsfiles, int cant) {
    //printf("------------------------------------------------------\n");
    int i;
    clearScanner();
    FILE * f;
    f = fopen("log.txt", "w");
    fprintf(f, "");
    fclose(f);

    codigo.sig = NULL;

    //Hacerlo con cada archivo
    int sinError = 1;
    for (i = 0; i < cant; i++) {
        /*FILE * f;
        f = fopen("log.txt", "a");
        fprintf(f, "%s\n", lsfiles[i]);
        fclose(f);*/
        fp = fopen(lsfiles[i], "r");
        archivoActual = lsfiles[i];
        if (fp == NULL) {
            return -1; //panico si no se puede abrir el archivo
        }

        ch = ' ';
        fin_de_archivo = 0;
        offset = -1;
        ll = 0;
        LineaActual = 0;
        PosicionLineaActual = 0;
        struct nodoArchivo *valor = NULL;
        buscarArchivoTDS(&valor, &tabla, lsfiles[i]);
        // //printf("Archivo %s \n", lsfiles[i]);
        if (valor == NULL) {
            ////printf("EL ARCHIVO NO SE REGISTRO BIEN PANICO TOTAL LA SEGUNDA PASADA PROBABLEMETE FUE EJECUTADA DE FORMA INDEPENDIENTO O SE CAMBIARON LA LISTA DE ARCHIVOS");
            log_error(40);
        }
        obtoken();
        int retorno = programa(valor, set_arranque);
        if (retorno == -1) {
            fclose(fp);
            fp = NULL;
            return -1;
        }
        //printf("Rgreso con %d\n", retorno);
        sinError = sinError & retorno;

        fclose(fp);
        fp = NULL;
    }
    return sinError;
}

void clearScanner() {
    primerError = 0;
    inicializar_espec();
    inicializarArbolPalabras();
    iniciarParamentros();
}

int programaP1(struct nodoArchivo *archivo, int toksig[]) {
    int sinErrores = 1;
    int vacio[NOTOKENS];
    init_set(vacio);

    do {
        if (token == incluirTok) {
            obtoken();
            if (token == datoCadena) {
                obtoken();
                if (token == puntoycoma) {
                    //guardar en la tabla
                    //printf("Se va a guardar en la tabla %s \n", valorCadena);
                    instarIncluidosArchivo(valorCadena, archivo);
                    obtoken();
                } else {
                    //log_error(14); // falto punto y coma;
                    sinErrores = 0;
                } //Se quita por que no se marcara errores y se perdonara la fatla de ;
            } else {
                int eIncluir[NOTOKENS];
                init_set(eIncluir);
                eIncluir[incluirTok] = 1;
                test(eIncluir, toksig, -1); //NO marcaremos errores el la primera pasada
                //log_error(15); // No esta bien escrita la cadena;
                //return 0;
                sinErrores = 0;
                if (token == -1) {
                    return -1;
                }
            }
        }
    } while (token == incluirTok);

    do {
        //Si es publica podria estar escribiendo
        //Un metodo o atributo en un mal lugar
        if (token == publicaTok || token == localTok || token == claseTok) {
            enum simbolo tokeAux;
            if (token == claseTok) {
                tokeAux = -1;
            } else {
                tokeAux = token;
                obtoken();
            }
            if (token == claseTok || token == ident || token == llaveI) {
                int tokenEntrada = token;
                //OK si es una clase
                if (token != ident && token != llaveI) {
                    obtoken();
                }

                if (token == ident || tokenEntrada == llaveI || token == llaveI) {
                    //y Esta decentemente escrita
                    // //printf("\tA guardar la clase %s\n", lex);
                    //Guardamos la definicion de clase
                    // Aunque pueda estar mal escrita
                    // Solo intereza su exitencia
                    struct clase *claseActual = NULL;
                    //printf("Clase %s\n", lex);
                    if (token == ident) {
                        insertarTDSClase(archivo, lex, tokeAux, &claseActual);
                    } else {
                        strcpy(lex, "1Clase");
                        insertarTDSClase(archivo, lex, tokeAux, &claseActual);
                    }

                    ////printf("Se inserto la clase %s con hash %d\n", claseActual->ident, claseActual->hash);
                    //Aunque no interese mucho que esta 100% bien 
                    // escrita si tiene que usar { cuerpo }
                    // para poder detectar todas las clases
                    // del mismo archivo
                    // Y evitar ambiguedades en el cuerpo
                    if (token != llaveI) {
                        obtoken();
                    }
                    if (token == llaveI || token == privadoTok || token == publicaTok) {
                        if (token != privadoTok && token != publicaTok) {
                            obtoken();
                        }
                        ////printf("\tRevisando el cuerpo\n");

                        int set_paso[NOTOKENS];
                        copia_set(set_paso, toksig);
                        set_paso[llaveF] = 1;

                        sinErrores = cuerpoP1(claseActual, set_paso);

                        if (sinErrores == -1) {
                            return -1;
                        }
                        if (token == llaveF) {
                            //printf("\tClase bien escrita\n");

                            obtoken();
                        } else {

                            test(toksig, vacio, -1);
                            sinErrores = 1;
                        }
                    } else {
                        test(toksig, vacio, -1);
                        sinErrores = 1;
                    }
                }
            }
        } else {
            if (token == privadoTok) {
                test(toksig, vacio, 56);
                //log_error(56);
                sinErrores = 0;
            } else {
                test(toksig, vacio, -1);
                sinErrores = 1;
            }
        }
        //printf("Repitiendo \n");
    } while (token != -1);

    //printf("SAliendo \n");
    return sinErrores;
}

int cuerpoP1(struct clase *clase, int toksig[]) {
    // Se aumentara cada vez que exista una llaver de apertura
    // Se disminuira cuando se cierre
    int saltar[NOTOKENS], vacio[NOTOKENS];
    init_set(saltar);
    init_set(vacio);
    saltar[publicaTok] = saltar[privadoTok] = 1;
    int countLlaveI = 1;
    if (token == llaveF) {
        ////printf("\tClase vacia no se avanzara un token\n");
        return 1;
    }
    do {
        if (token == publicaTok || token == privadoTok) {
            int alcanze = token;
            obtoken();
            char *nombre;
            int propiedad = 1;
            //Podria ser una propiedad o metodo
            if (token == ident) {
                //Se copia para no perderlo
                nombre = malloc(sizeof (char)* strlen(lex));
                strcpy(nombre, lex);
                obtoken();
                propiedad = 2;
                if (token == corcheteI) {
                    propiedad = 0;
                    //Ok es un metodo
                    // //printf("\t\tRegistrando metodo %s\n", nombre);
                    struct metodo *metodo;
                    metodo = (struct metodo*) malloc(sizeof (struct metodo));
                    metodo->ident = (char *) malloc(sizeof (char)*strlen(nombre) + 1);
                    metodo->locales = NULL;
                    metodo->alcance = alcanze;
                    strcpy(metodo->ident, nombre);
                    //printf("Insertando metodo %s\n", nombre);
                    insertarTDSMetodo(clase, metodo);

                    obtoken();
                    if (token == corcheteF) {
                        //metodo sin parametros
                        metodo->parametros = NULL;
                        ////printf("\t\t\t sin parametros \n");
                        obtoken();
                        if (token == arregloTok || token == objetoTok || token == archivoTok
                                || token == caracterTok || token == cadenaTok || token == enteroTok
                                || token == realTok || token == byteTok || token == booleanoTok || token == ident) {
                            ////printf("\t\t\t y con retorno %d\n", token);
                            metodo->tipoRetorno = token; //Cambiar esto por el tipo en mayusculas
                            metodo->esFuncion = TRUE;
                        } else {
                            if (token == llaveI) countLlaveI++;
                            if (token == llaveF) countLlaveI--;
                        }

                    } else {
                        //Tiene paramentros
                        //Preparamos el espacio para los paramentros
                        struct listaAtributo *parametros
                                = (struct listaAtributo *) malloc(sizeof (struct listaAtributo));
                        metodo->parametros = parametros;
                        metodo->parametros->sig = NULL;
                        int i = 0;
                        do {
                            struct atributo *parametroX = (struct atributo *) malloc(sizeof (struct atributo));
                            if (i == 1) {
                                obtoken();
                                //Es el 2 o mas
                                parametros->sig = (struct listaAtributo *) malloc(sizeof (struct listaAtributo));
                                parametros = parametros->sig;
                                parametros->sig = NULL;

                            }

                            parametros->atributo = parametroX;

                            // printf("Se detecto parametro de tipo\n");
                            if (!tipoDP1(parametroX, NULL)) {
                                log_error(27);
                                return -1;
                            }

                            if (token == por) {
                                // //printf(" es por referencia ");
                                parametroX->esPorReferencia = TRUE;
                                obtoken();
                            }

                            if (token == ident) {
                                parametroX->ident = (char *) malloc(sizeof (char) * strlen(lex) + 1);
                                strcpy(parametroX->ident, lex);
                                obtoken();
                                i = 1;
                                //printf("Se inserta el parametro %s\n", lex);
                            } else {
                                log_error(18); // Se esperaba un ident
                                return -1;
                                ////printf("Error \n");
                            }
                        } while (token == coma);

                        if (token == corcheteF) {
                            obtoken();
                            if (token == arregloTok || token == objetoTok || token == archivoTok
                                    || token == caracterTok || token == cadenaTok || token == enteroTok
                                    || token == realTok || token == byteTok || token == booleanoTok || token == ident) {
                                //printf("\t\t\t y con retorno %d\n", token);
                                metodo->tipoRetorno = token;
                                metodo->esFuncion = -1;
                            } else {
                                if (token == llaveI) countLlaveI++;
                                if (token == llaveF) countLlaveI--;
                            }


                            ////printf("\t\t\ty esta bien escrito\n");
                        } else {
                            log_error(31); // si esperaba corchete de cierre
                            return -1;
                        }


                    }


                }
            }

            //Es propiedad
            if (propiedad == 2 || propiedad == 1) {
                // //printf("\t\tPropiedad de tipo");

                struct atributo *atributo = (struct atributo*) malloc(sizeof (struct atributo));
                atributo->alcanze = alcanze;
                atributo->hashClase = 0;
                //ES un propieda cuyo tipo es un objeto
                if (propiedad == 2) {
                    //En la version completa buscar el hashClase
                    // //printf(" %s que es un objeto ", nombre);
                    atributo->esPrimitivo = 0;
                    atributo->tipo = OBJETO;

                } else {
                    if (!tipoDP1(atributo, NULL)) {
                        log_error(-1);
                        return -1;
                    }
                }
                //Para que verificar el tipo
                //Y ademas venga el inicio del registro
                //De tds


                if (token == ident) {
                    // //printf("El nombre es %s ", lex);
                    atributo->ident = (char *) malloc(sizeof (char)* strlen(lex) + 1);
                    strcpy(atributo->ident, lex);
                    //printf(">>>>>>>>>>>>>>>>>>>>>>> Guardando en TDS propiedad  %s -------------------- \n", lex);
                    insertarTDSAtributo(clase, atributo);
                    obtoken();
                    do {
                        //viene otra priopiedad 
                        // del mismo tipo
                        if (token == coma) {
                            obtoken();
                            //Y se llama
                            if (token == ident) {
                                struct atributo *atributo2 = (struct atributo*) malloc(sizeof (struct atributo));
                                atributo2->esPrimitivo = atributo->esPrimitivo;
                                atributo2->tipo = atributo->tipo;
                                atributo2->ident = (char *) malloc(sizeof (char)* strlen(lex) + 1);
                                atributo2->tipoContenidoArreglo = atributo->tipoContenidoArreglo;
                                atributo2->alcanze = atributo->alcanze;
                                atributo2->valor = atributo->valor; //copiarValor(atributo2,atributo); //No son copia es el mismo
                                //tenemos que hacer
                                atributo2->hashClase = atributo->hashClase;
                                strcpy(atributo2->ident, lex);
                                ////printf("\t\t luego esta %s ", lex);
                                //printf(">>>>>>>>>>>>>>>>>>>>>>> Guardando en TDS propiedad  %s <<<<<<<<<<<<<<<<<<<<<<<<<<< \n", lex);
                                insertarTDSAtributo(clase, atributo2);
                            } else {
                                log_error(18);
                                return -1;
                            }
                        }
                        if (token == puntoycoma) {
                            break;
                        }
                        obtoken();
                    } while (token != puntoycoma && token != -1);
                    if (token == -1) {
                        log_error(14);
                        return -1;
                    }

                } else {
                    log_error(18);
                    return -1;
                }
            }

        }
        obtoken();
        //printf("Otro metodo o atributo \n");
        if (token == llaveI) countLlaveI++;
        if (token == llaveF) countLlaveI--;

    } while (countLlaveI > 0 && token != -1);

    if (token == -1) {
        log_error(39);
        return -1;
    } else {
        return 1;
    }
}

int tipoDP1(struct atributo *atributo, int toksig[]) {
    atributo->esPrimitivo = 1;
    atributo->valor = NULL; // NO se crea la instaciona hasta que sea necesario
    switch (token) {
            //facil es un objeto;
        case ident: //Para este caso en la pasda1 intereza buscar el hash de la clase
            // Y hacer lo que hace obejo y añadir un numero de instancia
            //printf(" Personalizado ");
        case objetoTok:
            //printf(" Objeto ");
            atributo->esPrimitivo = 0;
            atributo->tipo = OBJETO;
            break;
        case archivoTok:
            //printf(" Archivo ");
            atributo->esPrimitivo = 0;
            atributo->tipo = ARCHIVO;
            break;
        case caracterTok:
            //printf(" Caracter ");
            atributo->tipo = CARACTER;
            break;
        case cadenaTok:
            //printf(" Cadena ");
            atributo->tipo = CADENA;
            break;
        case enteroTok:
            //printf(" Entero ");
            atributo->tipo = ENTERO;
            break;
        case realTok:
            // printf(" Real ");
            atributo->tipo = REAL;
            break;
        case byteTok:
            //printf(" Byte ");
            atributo->tipo = BYTE;
            break;
        case booleanoTok:
            //printf(" Booleano ");
            atributo->tipo = BOOLEANO;
            break;
        case arregloTok: //Caso dificil
            atributo->esPrimitivo = 0;
            ////printf(" Arreglo que almacena de tipo <");
            obtoken();
            if (token == mnr) {
                struct atributo *valor = (struct atributo*) malloc(sizeof (struct atributo));
                obtoken();
                if (!tipoDP1(valor, NULL)) {
                    return 0;
                }

                if (token == myr) {
                    atributo->esPrimitivo = 0;
                    atributo->tipo = ARREGLO;

                    if (valor->tipo == ARREGLO) {
                        //Bueno es un arrelgo de arreglo
                        atributo->tipoContenidoArreglo = ARREGLO;
                        atributo->valor = (void *) valor;
                    } else {
                        //Contiene un elememto basico wii
                        atributo->tipoContenidoArreglo = valor->tipo;
                    }
                    ////printf(">. ");
                } else {
                    log_error(26); //se esperaba cierre
                    return 0;
                }
            } else {
                log_error(25);
                return 0;
            }
            break;
        default:
            //printf("Error en tipo \n");
            log_error(27); //NO es un tipo de dato valido
            return 0;
    }
    obtoken();

    return 1;
}

void copiarValor(struct atributo *dest, struct atributo *org) {
    switch (org->tipo) {
        case ENTERO:
            dest->valor = malloc(sizeof (int));
            //Para que solo copie lo del int 
            int *valorO = (int *) org->valor;
            int *valorD = (int *) org->valor;
            *valorD = *valorO;
            break;
        case REAL:
            dest->valor = malloc(sizeof (float));
            //Para que solo copie lo del int 
            float *valorOf = org->valor;
            float *valorDf = org->valor;
            *valorDf = *valorOf;
            break;
        case BYTE:
            dest->valor = malloc(sizeof (char));
            //Para que solo copie lo del int 
            char *valorOB = org->valor;
            char *valorDB = org->valor;
            *valorDB = *valorOB;
            break;
        case BOOLEANO:
            dest->valor = malloc(sizeof (int));
            //Para que solo copie lo del int 
            int *valorOBo = org->valor;
            int *valorDBo = org->valor;
            *valorDBo = *valorOBo;
            break;
        case CADENA:
            dest->valor = malloc(sizeof (char) * strlen(org->valor) + 1);
            //Para que solo copie lo del int 
            char **valorOC = org->valor;
            char **valorDC = org->valor;
            strcpy(*valorDC, *valorOC);
            break;
        case CARACTER:
            dest->valor = malloc(sizeof (char));
            //Para que solo copie lo del int 
            char *valorOCa = org->valor;
            char *valorDCa = org->valor;
            *valorDCa = *valorOCa;
            break;
        case ARREGLO:
            if (org->tipoContenidoArreglo != ARREGLO) {
                //Caso facil
            } else {
                //caso dificil me recursion 
            }
            break;
        default:
            //FALTA DEFINIR objeto y archivo
            break;
    }
}

//Programa necesita saber en el contexto de que archivo esta

int programa(struct nodoArchivo* miArchivo, int toksig[]) {
    int sinErrores = 1;
    int vacio[NOTOKENS];
    init_set(vacio);
    //Area de inclusion 
    
    do {
        if (token == incluirTok) {
            obtoken();
            if (token == datoCadena) {
                obtoken();
                //verificamos que lo que quiere incluir existe
                struct nodoArchivo *incluir = NULL;
                buscarArchivoTDS(&incluir, &tabla, valorCadena);
                //Verificamos pro que le la primera pasada solo se registor
                //pero no se sabia si existia o no
                if (incluir == NULL) {
                    //printf("El archivo a incluir no se reconoce\n");
                    log_error(13);
                    sinErrores = 0;
                }
                //printf("######## A BUSCAR PUNTO Y COMA DE INCLUIR\n");
                if (token == puntoycoma) {
                    //printf("Incluir %s\n", valorCadena);
                    obtoken();
                } else {
                    //printf("######## A BUSCAR PUNTO Y COMA DE INCLUIR\n");
                    log_error(14); //se esperaba ;
                    sinErrores = 0;
                }
            } else {
                //log_error(15); //se espera cadena
                int eIncluir[NOTOKENS];
                init_set(eIncluir);
                eIncluir[incluirTok] = 1;
                ////printf("////////////// Estabilizando ///////\n");
                test(eIncluir, toksig, 15);
                // //printf("////////////// Estabilizado ///////\n");
                sinErrores = 0;

            }
        }
    } while (token == incluirTok);


    //Area de clase

    do {
        //printf("Actual \n");
        if (token == publicaTok || token == localTok) {
            int tokenAux = token;
            obtoken();
            if (token == claseTok) {
                int tokenAux2 = token;
                obtoken();

                if (token == ident) {
                    struct clase *yoClase = NULL;
                    obtenerClase(miArchivo, &yoClase, lex);

                    //Esatableciendo la clase en la que se esta escribiendo
                    struct claseCod clase; //Ya conozco la clase
                    //clase->hash = 0;
                    claseAct = &clase;

                    if (yoClase == NULL) {
                        //printf("Error clase no registrad en la primera pasada\n");
                        log_error(19);
                        sinErrores = 0;
                        clase.hash = -3;
                    } else {
                        clase.hash = yoClase->hash;
                    }
                  //printf(" ------------- %d %s %d-------------\n", yoClase->hash, yoClase->ident, yoClase->hash);

                    //verifica que la clase solo exita en este archivo
                    if (evitarRedefinicionClase(lex, miArchivo, &tabla) >= 1) {
                        //printf("Esta clase exite en mas de un lugar\n");
                        log_error(20); //la calse estaba definida
                        sinErrores = 0;
                        //printf("AAAA");
                    }
                    //printf("Buscando llave { clase \n");
                    obtoken();
                    if (token == llaveI) {
                        obtoken();


                        int sigCuerpo[NOTOKENS];
                        copia_set(sigCuerpo, toksig);
                        sigCuerpo[llaveF] = 1;
                        //printf("Examinando el cuerpo\n");

                        if (!cuerpo(miArchivo, yoClase, toksig)) {
                            sinErrores = 0;
                        }
                        if (token == llaveF) {
                            //                            clase.
                            clase.hash = yoClase->hash;
                            addClase(clase, &codigo);
                            //printf("-------------Clase bien escrita-----------------\n");
                            obtoken();
                        } else {
                            sinErrores = 0;
                            test(toksig, vacio, 21);
                        }
                    } else {
                        sinErrores = 0;
                        test(toksig, vacio, 21);
                    }
                } else {
                    test(toksig, vacio, 18);
                    sinErrores = 0;
                }
            } else {
                //log_error(17); //Se esperaba la declaracion de un clase
                //printf("Hola");
                test(toksig, vacio, 17);
                sinErrores = 0;
            }
        } else {
            //log_error(56);
            test(toksig, vacio, 56);
            sinErrores = 0;
            //printf("\n 1-------------------- Saltando --------------- 2\n");
        }
        //printf("Sig Ejecucion program\n");
    } while (token != -1);

    //printf("Terminadno program con %d \n", sinErrores);
    return sinErrores;
}

int cuerpo(struct nodoArchivo* miArchivo, struct clase *clase, int toksig[]) {
    int sinErrores = 1;
    int vacio[NOTOKENS];

    init_set(vacio);

    if (token == llaveF) {
        //printf("\t No tenia nada");
        return 1;
    }

    struct lsCod atributos; // Codigo para instanciar los atributos
    atributos.sig = NULL;
    atributos.instruccion = genIns(INS, -6, -6, -6);
    struct groupCode metodos; //Codigo que se encutara en llamada a metodos
    metodos.sig = NULL;
    metodos.value.instruccion = genIns(INS, -6, -6, -6);
    metodos.groupNum = -1;
    int numAtr = 0;
    //int numMetodo = 1;

    do {
        //printf("En cueropo\n");
        if (token == publicaTok || token == privadoTok) {
            //printf("\tDeclaracion de atributo o metodo\n");
            obtoken();
            //Se sabe con facilidad que es un atributo
            if (token == arregloTok || token == objetoTok || token == archivoTok
                    || token == caracterTok || token == cadenaTok || token == enteroTok
                    || token == realTok || token == byteTok || token == booleanoTok) {
                //Se llama realmente porque puede ser un arreglo 
                int sigTipo[NOTOKENS];
                copia_set(sigTipo, toksig);

                sigTipo[puntoycoma] = sigTipo[coma] = 1;

                int valor = tipo(miArchivo, clase, sigTipo);
                sinErrores = sinErrores & valor;
                if (token == ident) {
                    struct atributo *atr = NULL;
                    buscarAtributo(&atr, clase, lex);



                    if (atr == NULL) {
                        //printf("--------------Aqui-----------\n");
                        //printf("\t\t Se esta redefiniedo el atributo %s\n", lex);
                        log_error(23); //este atributo no se detecto en la primera pasada
                        sinErrores = 0;
                    } else {

                        atr->atributoNum = numAtr;
                        genInstacia(&numAtr, atr, &atributos);
                      //printf("Para %s tiene de numero atr %d", atr->ident, atr->atributoNum);
                    }



                    //printf("\t\tSe Declaro con exito %s\n", atr->ident);
                    obtoken();
                    if (token == asignacion) {
                        //printf("\t\tSe le esta asignando un valor a %s \n", atr->ident);
                        obtoken();
                        int sigExpresion[NOTOKENS];
                        copia_set(sigExpresion, toksig);
                        sigExpresion[puntoycoma] = sigExpresion[coma] = 1;

                        lsCodActual = &atributos;
                        if (!expresion(miArchivo, clase, NULL, sigExpresion)) {
                            sinErrores = 0;
                        }

                        addLsCode(genIns(ALM, 0, 4, atr->atributoNum), lsCodActual);

                    }
                    int segundaVez = 0;
                    do {
                        if (token == coma) {
                            obtoken();
                            if (token == ident) {
                                struct atributo *atrAux = NULL;
                                buscarAtributo(&atrAux, clase, lex);
                                if (atrAux == NULL) {
                                    //printf("\t\t Se esta redefiniedo el atributo %s\n", lex);
                                    log_error(23); //este atributo no se detecto en la primera pasada 
                                    sinErrores = 0;
                                } else {
                                    atrAux->atributoNum = numAtr;
                                    genInstacia(&numAtr, atrAux, &atributos);
                                  //printf("Para %s tiene de numero atr %d", atrAux->ident, atrAux->atributoNum);
                                }
                                //printf("\t\tSe Declaro con exito %s\n", atrAux->ident);
                                obtoken();
                                if (token == asignacion) {
                                    //printf("\t\tSe le esta asignando un valor a %s \n", atrAux->ident);
                                    obtoken();
                                    int sigExpresion[NOTOKENS];
                                    copia_set(sigExpresion, toksig);
                                    sigExpresion[puntoycoma] = sigExpresion[coma] = 1;

                                    lsCodActual = &atributos;

                                    if (!expresion(miArchivo, clase, NULL, sigExpresion)) {
                                        sinErrores = 0;
                                    }

                                    addLsCode(genIns(ALM, 0, 4, atrAux->atributoNum), lsCodActual);
                                }
                                /* } else {
                                     //printf("\t\t Se esta redefiniedo el atributo %s\n", lex);
                                     log_error(23); //este atributo no se detecto en la primera pasada 
                                     sinErrores = 0;
                                 }*/
                            } else {
                                int sigDeclaracion[NOTOKENS];
                                init_set(sigDeclaracion);
                                sigDeclaracion[coma] = sigDeclaracion[puntoycoma] = 1;
                                test(toksig, sigDeclaracion, 18);
                                //log_error(18); //se esperaba un idnet de atributo
                                sinErrores = 0;
                            }

                        }
                        if (token == puntoycoma || (segundaVez == 1 && token != puntoycoma && token != coma)) {
                            break;
                        }
                        segundaVez = 1;
                    } while (token != -1);

                    if (token == puntoycoma) {
                        obtoken();
                        //printf("----------Linea de declaracion bien escrita \n------");
                    } else {
                        //printf("Falta ; \n");
                        test(toksig, vacio, 14);
                        //log_error(14);
                        sinErrores = 0;
                    }
                    /*} else {
                        //printf("\t\t Se esta redefiniedo el atributo %s\n", lex);
                        log_error(23); //este atributo no se detecto en la primera pasada
                        sinErrores = 0;
                    }*/
                } else {
                    test(toksig, vacio, 18);
                    //log_error(18); //atributo mal escrito
                    sinErrores = 0;
                }
            } else {
                int sigTipo[NOTOKENS];
                copia_set(sigTipo, toksig);
                sigTipo[puntoycoma] = 1;
                char claseAtributo[MAX_LINEA];
                strcpy(claseAtributo, lex);
                //printf("Fallando \n");
                if (tipo(miArchivo, clase, sigTipo) == 1) {
                    if (token == ident) {
                        //printf("FAllando 2\n");
                        struct atributo *atr = NULL;
                        buscarAtributo(&atr, clase, lex);
                        if (atr == NULL) {
                            //printf("\t\tSe esta redefiniedo el atributo %s\n", lex);
                            log_error(23); //este atributo no se detecto en la primera pasada
                            sinErrores = 0;
                        } else {

                            struct clase* miClase = NULL;
                            buscarClaseTDS(&miClase, &tabla, claseAtributo);
                            if (miClase != NULL) {
                                atr->hashClase = miClase->hash;
                                // printf("Atributo detectado clase hash %d\n" ,atr->hashClase);
                                atr->atributoNum = numAtr;
                                genInstacia(&numAtr, atr, &atributos);
                              //printf("Para %s tiene de numero atr %d", atr->ident, atr->atributoNum);
                                //printf("----------------------------------------- guardado has ---------\n");
                                //hash atributo
                            } else {
                                //printf("\t\tSe esta redefiniedo el atributo %s\n", lex);
                                log_error(58); //este atributo no se detecto en la primera pasada
                                sinErrores = 0;
                            }
                        }
                        //printf("\n\t\tSe Declaro con exito %s (objeto)\n", atr->ident);
                        obtoken();
                        if (token == asignacion) {
                            obtoken();
                            int sigExpresion[NOTOKENS];
                            copia_set(sigExpresion, toksig);
                            sigExpresion[puntoycoma] = sigExpresion[coma] = 1;
                            lsCodActual = &atributos;
                            if (!expresion(miArchivo, clase, NULL, sigExpresion)) {
                                sinErrores = 0;
                            }
                            addLsCode(genIns(ALM, 0, 4, atr->atributoNum), lsCodActual);
                        }

                        do {
                            if (token == coma) {
                                obtoken();
                                if (token == ident) {
                                    struct atributo *atrAux = NULL;
                                    buscarAtributo(&atrAux, clase, lex);
                                    if (atrAux == NULL) {
                                        //printf("\t\t Se esta redefiniedo el atributo %s\n", lex);
                                        log_error(23); //este atributo no se detecto en la primera pasada 
                                        sinErrores = 0;
                                    } else {
                                        struct clase* miClase = NULL;
                                        buscarClaseTDS(&miClase, &tabla, claseAtributo);
                                        if (miClase != NULL) {
                                            atrAux->hashClase = miClase->hash;
                                            // printf("1Atributo detectado clase hash %d\n" ,atr->hashClase);
                                            atrAux->atributoNum = numAtr;
                                            genInstacia(&numAtr, atrAux, &atributos);
                                          //printf("Para %s tiene de numero atr %d", atrAux->ident, atrAux->atributoNum);
                                            //printf("--------------Guardandole el hash\n");
                                            //hash atributo
                                        } else {
                                            //printf("\t\tSe esta redefiniedo el atributo %s\n", lex);
                                            log_error(58); //este atributo no se detecto en la primera pasada
                                            sinErrores = 0;
                                        }
                                    }
                                    //printf("\t\tSe Declaro con exito %s\n", atrAux->ident);
                                    obtoken();
                                    if (token == asignacion) {
                                        //printf("\t\tSe le esta asignando un valor a %s \n", atrAux->ident);
                                        obtoken();
                                        int sigExpresion[NOTOKENS];
                                        copia_set(sigExpresion, toksig);
                                        sigExpresion[puntoycoma] = sigExpresion[coma] = 1;
                                        lsCodActual = &atributos;
                                        if (!expresion(miArchivo, clase, NULL, sigExpresion)) {
                                            sinErrores = 0;
                                        }
                                        
                                        addLsCode(genIns(ALM, 0, 4, atrAux->atributoNum), lsCodActual);
                                    }
                                    /* } else {
                                         //printf("\t\t Se esta redefiniedo el atributo %s\n", lex);
                                         log_error(23); //este atributo no se detecto en la primera pasada 
                                         sinErrores = 0;
                                     }*/
                                } else {
                                    int sigDeclaracion[NOTOKENS];
                                    init_set(sigDeclaracion);
                                    sigDeclaracion[coma] = sigDeclaracion[puntoycoma] = 1;
                                    test(toksig, sigDeclaracion, 18);
                                    //log_error(18); //se esperaba un idnet de atributo
                                    sinErrores = 0;
                                }
                            }
                            if (token == puntoycoma) {
                                break;
                            }
                        } while (token != -1);

                        if (token == puntoycoma) {
                            obtoken();
                            //printf("----------Linea de declaracion bien escrita (Objeto) \n------------------");
                        } else {
                            test(toksig, vacio, 14);
                            //log_error(14);
                            sinErrores = 0;
                        }
                        /*} else {
                            //printf("\t\tSe esta redefiniedo el atributo %s\n", lex);
                            log_error(23); //este atributo no se detecto en la primera pasada
                            sinErrores = 0;
                        }*/

                    } else {
                        test(toksig, vacio, 18);
                        //  log_error(18); //se esperava un ident para el atriburo
                        sinErrores = 0;
                    }
                } else {



                    //printf("\t\t---------Definicion de metodo-------------------- \n");
                    int sigMetodo[NOTOKENS];
                    copia_set(sigMetodo, toksig);
                    sigMetodo[llaveI] = 1;


                    struct lsCod metodoCod;
                    metodoCod.sig = NULL;
                    metodoCod.instruccion = genIns(INS, -6, -6, -6);

                    lsCodActual = &metodoCod;
                    int lineaAnt = numeroLinea;
                    numeroLinea = 0;
                    if (!metodo(miArchivo, clase, sigMetodo)) {
                        sinErrores = 0;
                        //printf("Retornando con error\n");
                    }
                    numeroLinea = lineaAnt;
                    addGroupCode(metodoCod, &metodos);
                    //printf("\t\t---------Definicio de metodo completa---------------- \n");
                }

            }
        } else {
            int sigCuerpo[NOTOKENS];
            init_set(sigCuerpo);
            sigCuerpo[llaveF] = 1;
            test(toksig, sigCuerpo, 16);
            sinErrores = 0;
            //printf("Hola---");
        }

        if (token == llaveF) {
            break;
        }

    } while (token != -1);


    claseAct->atributos = atributos;
    addLsCode(genIns(OPR, 0, -1, 0), &atributos);
    claseAct->metodos = metodos;
    //printf("Terminando cuerpo con %d\n", sinErrores);
    return sinErrores;
}

int tipo(struct nodoArchivo *miArchivo, struct clase *clase, int toksig[]) {

    int vacio[NOTOKENS];
    init_set(vacio);
    int sinError = 1;
    switch (token) {

        case ident:
            //printf("A preguntar por tipo obejto %s\n", lex);
            if (puedoUsarEsteTipo(lex, miArchivo, clase, &tabla) == 1) {
                obtoken();
                //printf("Tipo Adecuado\n");
                return 1;
            } else {
                //printf("Tipo NO Adecuado\n");
                return 0;
            }
            break;
        case objetoTok:
        case archivoTok:
        case caracterTok:
        case cadenaTok:
        case enteroTok:
        case realTok:
        case byteTok:
        case booleanoTok:
            //printf("ES de tipo primitivo");
            obtoken();
            return 1;
            break;
        case arregloTok:

            //printf("Se declara un arreglo de tipo < ");
            obtoken();
            if (token == mnr) {
                obtoken();
                int sigTipoR[NOTOKENS];
                copia_set(sigTipoR, toksig);
                sigTipoR[myr] = 1;
                if (!tipo(miArchivo, clase, sigTipoR)) {
                    sinError = 0;
                }
                if (token == myr) {
                    obtoken();
                    //printf(" >\n");

                } else {
                    test(toksig, vacio, 26);
                    //log_error(26); //se espera simbolo de mayor
                }

            } else {
                test(toksig, vacio, 25);
                //log_error(25); //se espera simbolo de menor
                sinError = 0;
            }
            return sinError;
            break;
        case llaveI: //especial para retorno

            return 2;
        case llaveF:
            return 3;
        default:
            if (toksig[0] != -1) {
                test(toksig, vacio, 27);
            }
            //log_error(27); //NO es un tipo de dato valido
            return 0;
    }
    return 0;
}

int metodo(struct nodoArchivo* miArchivo, struct clase *clase, int toksig[]) {
    int sinError = 1;
    int vacio[NOTOKENS];
    int posLocales = 1;
    init_set(vacio);
    if (token == ident) {
        struct metodo *metodo = NULL;
        buscarMetodo(&metodo, clase, lex);
        if (metodo == NULL) {
            //printf("Se esta intentando redefinir el metodo %s\n", lex);
            //printf("primerr %d\n", primerError);
            log_error(28); // declaracion mal escrita
            sinError = 0;
            //printf("primerr %d\n", primerError);
        } else {
            addLsCode(genIns(INS, 0, 4, 0), lsCodActual);
        }
        obtoken();
        if (token == corcheteI) {
            obtoken();
            if (token == corcheteF) {
                int sigTipo[NOTOKENS];
                copia_set(sigTipo, toksig);
                toksig[llaveI] = 1;
                obtoken();
                int tokenAux = token;

                //printf("---->---------> %d\n", tokenAux);
                int rs = tipo(miArchivo, clase, sigTipo);
                if (rs == 1) {
                    //tiene rotorno verificar si es el mismo
                    if (metodo->tipoRetorno != tokenAux) {
                        //printf("Tipo de retono inesperado\n");
                        int sigMetodo[NOTOKENS];
                        init_set(sigMetodo);
                        sigMetodo[llaveI] = 1;
                        test(toksig, sigMetodo, 30); // se esperaba cierre de corchete
                        sinError = 0;
                    }
                }
                //printf("Metodo %s bien declarado \n", metodo->ident);
                if (rs == 0) {
                    obtoken();
                }
                if (!bloque(miArchivo, clase, metodo, toksig)) {
                    sinError = 0;
                }

                addLsCode(genIns(OPR, 0, 0, 0), lsCodActual);
            } else {
                int sigParametro[NOTOKENS];
                copia_set(sigParametro, toksig);
                sigParametro[corcheteF] = sigParametro[coma] = 1;
                numActual = posLocales;
                if (!parametro(miArchivo, clase, metodo, sigParametro)) {
                    sinError = 0;
                }

                addLsCode(genIns(ALM, 0, 4, posLocales), lsCodActual);
                posLocales++;

                while (token == coma) {
                    obtoken();
                    numActual = posLocales;
                    if (!parametro(miArchivo, clase, metodo, sigParametro)) {
                        sinError = 0;
                    }

                    addLsCode(genIns(ALM, 0, 4, posLocales), lsCodActual);
                    posLocales++;
                }
                if (token == corcheteF) {
                    obtoken();
                    int sigTipo[NOTOKENS];
                    copia_set(sigParametro, toksig);
                    sigTipo[llaveI] = 1;
                    int tokenAux = token;
                    int rs = tipo(miArchivo, clase, sigTipo);
                    if (rs == 1) {
                        //tiene rotorno verificar si es el mismo
                        if (metodo->tipoRetorno != tokenAux) {
                            //printf("Tipo de retono inesperado");
                            log_error(30);
                            sinError = 0;
                        }
                    }
                    ////printf("Metodo %s con parametros bien declarado \n", metodo->ident);

                    if (rs == 0) {
                        obtoken();
                    }
                    numActual = posLocales;
                    if (!bloque(miArchivo, clase, metodo, toksig)) {
                        sinError = 0;
                    }

                    addLsCode(genIns(OPR, 0, -2, 0), lsCodActual);
                } else {
                    test(toksig, vacio, 31); // se esperaba cierre de corchete
                    sinError = 0;
                }

            }
        } else {
            test(toksig, vacio, 19);
            //log_error(29);
            sinError = 0;
        }
        /*} else {
            //printf("Se esta intentando redefinir el metodo %s\n", lex);
            log_error(28); // declaracion mal escrita
            return 0;
        }*/
    } else {
        test(toksig, vacio, 18);
        //log_error(18);
        sinError = 0;
    }
    return sinError;
}

int parametro(struct nodoArchivo *miArchivo, struct clase *clase, struct metodo *metodo, int toksig[]) {
    int sinError = 1;

    int sigTipo[NOTOKENS];
    copia_set(sigTipo, toksig);
    sigTipo[por] = sigTipo[ident] = 1;
    int referencia = 0;

    if (tipo(miArchivo, clase, sigTipo) == 1) {

        if (token == por) {
            obtoken();
            referencia = 1;
        }

        if (token == ident) {
            //printf("Parametro %s detectado\n", lex);
            if (1 == sePuedeUsarComoParametro(clase, metodo, lex)) {
                struct atributo *atr = NULL;
                buscarAtributo2(&atr, metodo->parametros, lex);
                atr->atributoNum = numActual;
                if (referencia) {
                    addLsCode(genIns(CRR, 2, 0, numActual), lsCodActual);
                } else {
                    addLsCode(genIns(CAR, 2, 0, numActual), lsCodActual);
                }
                obtoken();
            } else {
                log_error(41); //se esta redefiniedo parametro
                sinError = 0;
            }
        } else {
            sinError = 0;
            int vacio[NOTOKENS];
            test(toksig, vacio, 18);
            //log_error(18); // Se esperaba un ident

            //return 0;
        }
    }
    return sinError;
}

int bloque(struct nodoArchivo* miArchivo, struct clase *clase, struct metodo *metodo, int toksig[]) {

    int sinError = 1;
    int vacio[NOTOKENS];

    init_set(vacio);

    //printf("Entrando a bloque\n");
    if (token == llaveI) {
        obtoken();
        int seguir = 1;
        do {

            do {
                int rs;
                char *claseName = (char *) malloc(sizeof (char)* strlen(lex) + 1);
                strcpy(claseName, lex);
                //printf("Lex leido %s\n");
                int tokenTipo = token;
                int trampa[1];
                trampa[0] = -1;

                rs = tipo(miArchivo, clase, trampa);


                if (rs == 0) {
                    seguir = 0;
                    break;
                }
                if (rs == 3) {
                    seguir = -1;
                    break;
                }
                if (rs == 1) {
                    if (token == ident) {
                        if (evitarRedefinicionBloque(clase, metodo, lex) > 1) {
                            //printf("Esta intentando redefinir un parametro o atributo\n");
                            log_error(41);
                            sinError = 0;
                        }
                        //printf("\n\n\nSe detecto declaracion");
                        struct atributo *atr = (struct atributo *) malloc(sizeof (struct atributo));
                        atr->tipo = tokenTipo;
                        atr->esPrimitivo = 1;
                        if (tokenTipo == ident) {
                            atr->tipo = OBJETO;
                            atr->esPrimitivo = 0;
                            struct clase *clasePapa = NULL;
                            buscarClaseTDS(&clasePapa, &tabla, claseName);
                            if (clasePapa == NULL) {
                                //printf("NO se encontro una clase\n");
                                log_error(19);
                                sinError = 0;
                            } else {
                                atr->hashClase = clasePapa->hash;
                            }
                        }
                        atr->ident = (char *) malloc(sizeof (char) * strlen(lex) + 1);
                        strcpy(atr->ident, lex);

                        //SI es de tipo objeto ir a buscar el hash de la clase
                        //Y asignarle un hasd de instancia
                        insertarDefinicionLocal(metodo, atr);
                        atr->atributoNum = numActual;
                        genInstacia(&numActual, atr, lsCodActual);
                      //printf("Para %s tiene de numero atr %d", atr->ident, atr->atributoNum);

                        obtoken();
                        if (token == asignacion) {
                            obtoken();
                            int sigExpresion[NOTOKENS];
                            copia_set(sigExpresion, toksig);
                            sigExpresion[coma] = 1;
                            if (!expresion(miArchivo, clase, metodo, sigExpresion)) {
                                sinError = 0;
                            }

                            addLsCode(genIns(ALM, 0, 4, atr->atributoNum), lsCodActual);
                        }

                        while (token == coma) {
                            obtoken();
                            if (token == ident) {
                                if (evitarRedefinicionBloque(clase, metodo, lex) > 1) {
                                    //printf("Esta intentando redefinir un parametro o atributo\n");
                                    log_error(41);
                                    sinError = 0;
                                }
                                struct atributo *atr = (struct atributo *) malloc(sizeof (struct atributo));
                                atr->tipo = tokenTipo;
                                atr->esPrimitivo = 1;
                                if (tokenTipo == ident) {
                                    atr->tipo = OBJETO;
                                    atr->esPrimitivo = 0;
                                }
                                atr->ident = (char *) malloc(sizeof (char) * strlen(lex) + 1);
                                strcpy(atr->ident, lex);

                                //SI es de tipo objeto ir a buscar el hash de la clase
                                //Y asignarle un hasd de instancia
                                insertarDefinicionLocal(metodo, atr);
                                atr->atributoNum = numActual;
                                genInstacia(&numActual, atr, lsCodActual);
                              //printf("Para %s tiene de numero atr %d", atr->ident, atr->atributoNum);
                                //printf(" Se detecto declaracion misma linea");
                                obtoken();
                                if (token == asignacion) {
                                    obtoken();
                                    int sigExpresion[NOTOKENS];
                                    copia_set(sigExpresion, toksig);
                                    sigExpresion[coma] = 1;
                                    if (!expresion(miArchivo, clase, metodo, sigExpresion)) {
                                        sinError = 0;
                                    }

                                    addLsCode(genIns(ALM, 0, 4, atr->atributoNum), lsCodActual);
                                }
                                //printf("\n");
                                /*} else {
                                    //printf("Esta intentando redefinir un parametro o atributo\n");
                                    log_error(41);
                                    return 0;
                                }*/
                            } else {
                                int otraVar[NOTOKENS];
                                otraVar[coma] = 1;
                                test(toksig, otraVar, 18);
                                //log_error(18);
                                sinError = 0;
                            }
                        }
                        /* } else {
                             //printf("Esta intentando redefinir un parametro o atributo\n");
                             log_error(41);
                             return 0;
                         }*/
                    } else {
                        int sigDecla[NOTOKENS];
                        sigDecla[puntoycoma] = 1;
                        test(toksig, sigDecla, 18);
                        //log_error(18);
                        sinError = 0;
                        //return 0;
                    }
                }
            } while (token != puntoycoma);
            if (seguir == -1 || seguir == 0) {
                break;
            }
            obtoken();
        } while (seguir == 1);

        int sigIns[NOTOKENS];
        copia_set(sigIns, toksig);
        sigIns[llaveF] = 1;

        if (!instruccion(miArchivo, clase, metodo, sigIns)) {
            sinError = 0;
        }
        if (token == llaveF) {
            //printf("Metodo OK\n");
            obtoken();
        } else {
            test(toksig, vacio, 22);
            //log_error(22); //se esperaba una llave de cierre
            sinError = 0;
        }
    } else {
        test(toksig, vacio, 21);
        //log_error(21); //se esperaba una llave de apertura
        sinError = 0;
    }
    return sinError;
}

int instruccion(struct nodoArchivo* miArchivo, struct clase *clase, struct metodo *metodo, int toksig[]) {
    int es0 = 0, sinError = 1;
    int sig[NOTOKENS];
    union_set(sig, toksig, set_ins);

    struct atributo *atr;
    switch (token) {
        case ident:
            //printf("es metodo %s\n", lex);

            if (esMetodo(clase, lex)) {
                //printf("Metodo Encontrado llamada a mi mismo metodo\n");
                int sigLlaMet[NOTOKENS];
                copia_set(sigLlaMet, sig);
                sigLlaMet[puntoycoma] = 1;
                if (!llamada_metodo(miArchivo, clase, metodo, sigLlaMet)) {
                    sinError = 0;
                }
                //printf("Metodo Encontrado salida a mi mismo metodo\n");
                if (token == puntoycoma) {
                    obtoken();
                } else {
                    test(set_ins, toksig, 14);
                    sinError = 0;
                }
                break;
            }

            //printf("es objeto %s\n", lex);
            int local;
            es0 = esObjeto(clase, metodo, lex, &atr, &local);
            if (es0) obtoken();
            //printf("Llegamos al punto \n");
            //Es una llamada a metodo de una clase
            if (token == punto) {
                //printf("Entrando al punto\n");
                if (atr->tipo == ARCHIVO) {
                    int sigFa[NOTOKENS];
                    copia_set(sigFa, toksig);
                    sigFa[puntoycoma] = 1;
                    obtoken();
                    if (!funcion_archivo(miArchivo, clase, metodo, sigFa)) {
                        sinError = 0;
                    }
                    if (token == puntoycoma) {
                        obtoken();
                    } else {
                        test(set_ins, toksig, 14);
                        //log_error(14); //se eperaba punto y coma
                        sinError = 0;
                    }
                } else {
                    //if(es0){
                    //printf("Llamada a metodo o atributo\n");
                    obtoken();
                    struct clase *miClase = NULL;
                    //printf("HashClaes %d \n", atr->hashClase);

                    buscar_def_clase_hash(&miClase, atr->hashClase, &tabla); //regresa la def de la clase
                    if (miClase == NULL) {
                        test(toksig, sig, 55);
                        sinError = 0;
                        break;
                    }

                    //printf("Es de la clase %s\n", clase->ident);
                    //busca dentro de ella si es metodo o es atributo
                    //printf("------------- Es llamada a metodo \n");
                    if (esMetodo(miClase, lex)) {
                        obtoken();
                        //printf("Metodo Encontrado llamada a mi mismo metodo\n");
                        int sigLlaMet[NOTOKENS];
                        copia_set(sigLlaMet, sig);
                        sigLlaMet[puntoycoma] = 1;

                        struct metodo *llamadoMetodo = NULL;
                        buscarMetodo(&llamadoMetodo, miClase, lex);

                        if (llamadoMetodo->alcance == privadoTok) {
                            if (miClase->hash != clase->hash) {
                                log_error(61);
                                sinError = 0;
                            }
                        }

                        if (!llamada_metodo(miArchivo, clase, metodo, sigLlaMet)) {
                            sinError = 0;
                        }
                        //printf("Metodo Encontrado salida a mi mismo metodo\n");
                        if (token == puntoycoma) {
                            obtoken();
                        } else {
                            test(set_ins, toksig, 14);
                            sinError = 0;
                        }
                        break;
                    } else {
                        //printf("------------- Es atributo \n");
                        int esatr = esAtributo(miClase, lex);

                        if (!esatr) {
                            log_error(59);
                            sinError = 0;
                        } else {
                            struct atributo*atr = NULL;
                            buscarAtributo(&atr, miClase, lex);
                            if (atr->alcanze == privadoTok) {
                                log_error(61);
                                sinError = 0;
                            }

                        }
                        obtoken();
                        if (token == asignacion) {
                            //printf("Se esta haciendo una asignacion\n");
                            obtoken();
                            int expresionSig[NOTOKENS];
                            union_set(expresionSig, toksig, set_ins);
                            expresionSig[puntoycoma] = 1;
                            if (!expresion(miArchivo, clase, metodo, expresionSig)) {
                                sinError = 0;
                            }

                            addLsCode(genIns(ALM, local, 4, atr->atributoNum), lsCodActual);
                        }
                        if (token == puntoycoma) {
                            obtoken();
                        } else {
                            if (esatr == 1 && token == corcheteI) {
                                test(set_ins, toksig, 60);
                            } else {
                                test(set_ins, toksig, 14);
                            }
                            sinError = 0;
                        }

                        break;
                    }

                }
            } else {
                if (!es0) {
                    //es una variable normal
                    //printf("Asignacion a una varaible \n");
                } else {
                    //es un objeto
                    //printf("Asignacion a un objeto\n");
                }
                obtoken();
                //Es una asignación
                if (token == asignacion) {
                    //printf("Se esta haciendo una asignacion\n");
                    obtoken();
                    int expresionSig[NOTOKENS];
                    union_set(expresionSig, toksig, set_ins);
                    expresionSig[puntoycoma] = 1;
                    if (!expresion(miArchivo, clase, metodo, expresionSig)) {
                        sinError = 0;
                    }
                    if (token == puntoycoma) {
                        obtoken();
                        //printf("EStaba escrita bien la asgnacion\n");
                    }
                    addLsCode(genIns(ALM, local, 4, atr->atributoNum), lsCodActual);
                } else {
                    obtoken();
                    log_error(32); //esperaba una llaamda a metodo o un asignacion;
                    sinError = 0;

                }
            }
            break;
        case romperTok:

            if (!instruccion_romper(sig)) {
                sinError = 0;
            }
            //printf("SE encontro romper\n");
            break;
        case siTok:
            if (!instruccion_si(miArchivo, clase, metodo, sig)) {
                sinError = 0;
            }
            ////printf("SE encontro si\n");
            break;
        case probarTok:
            if (!instruccion_probar(miArchivo, clase, metodo, sig)) {
                sinError = 0;
            }
            break;
        case mientrasTok:
            if (!instruccion_mientras(miArchivo, clase, metodo, sig)) {
                sinError = 0;
            }
            break;
        case paraTok:
            if (!instruccion_para(miArchivo, clase, metodo, sig)) {
                sinError = 0;
            }
            break;
        case hacerTok:
            if (!instruccion_hacer(miArchivo, clase, metodo, sig)) {
                sinError = 0;
            }
            break;
        case paracadaTok:
            if (!instruccion_paraCada(miArchivo, clase, metodo, sig)) {
                sinError = 0;
            }
            break;
        case sistemaTok:
            if (!instruccion_es(miArchivo, clase, metodo, sig)) {
                sinError = 0;
            }
            break;
        case retornarTok:
            obtoken();
            int sigExpresion[NOTOKENS];
            copia_set(sigExpresion, sig);
            sigExpresion[puntoycoma] = 1;
            if (!expresion(miArchivo, clase, metodo, sigExpresion)) {
                sinError = 0;
            }
            if (token == puntoycoma) {
                //printf("REtornar bien escrito");
                obtoken();
            } else {
                test(toksig, set_ins, 14);
                //log_error(14); //se esperaba punto y coma
                sinError = 0;
            }
            break;
        default:
            return 1;
            break;

    }
    //printf("MAs instrucciones\n");
    if (!instruccion(miArchivo, clase, metodo, sig)) {
        sinError = 0;
    }
    return sinError;
}

int instruccion_si(struct nodoArchivo* miArchivo, struct clase *clase, struct metodo *metodo, int toksig[]) {
    int sinError = 1;
    int vacio[NOTOKENS];
    init_set(vacio);
    if (token == siTok) {
        //printf("se encontro sitok\n");
        obtoken();
        if (token == corcheteI) {
            //printf("se encontro corcheteI\n");
            obtoken();
            //printf("a entrar a expresion numerica\n");
            int sigExpresion[NOTOKENS];
            copia_set(sigExpresion, toksig);
            sigExpresion[coma] = sigExpresion[corcheteF] = 1;
            if (!expresion(miArchivo, clase, metodo, sigExpresion)) {
                sinError = 0;
            }

            addLsCode(genIns(SAC, 0, 0, -1), lsCodActual);
            struct ins* sac = NULL;
            lastReferecence(&sac, lsCodActual);
            //int numeroPat = numeroLinea;

            if (token == coma) {
                //printf("\nEncontre una coma\n");
                obtoken();
                int sigAsignacion[NOTOKENS];
                copia_set(sigAsignacion, toksig);
                sigAsignacion[coma] = 1;
                if (!asignacionf(miArchivo, clase, metodo, sigAsignacion)) {
                    sinError = 0;
                }

                addLsCode(genIns(SAL, 0, 0, -1), lsCodActual);
                struct ins* sal = NULL;
                lastReferecence(&sal, lsCodActual);
                //printf("\nRegrese de asignacion en si corto\n");
                if (token == coma) {
                    //printf("\nEncontre segunda coma\n");
                    obtoken();
                    sigAsignacion[coma] = 0;
                    sigAsignacion[corcheteF] = 1;
                    if (!asignacionf(miArchivo, clase, metodo, sigAsignacion)) {
                        sinError = 0;
                    }

                    sal->p = numeroLinea;
                    sac->p = numeroLinea;

                    //printf("\nRegrese de 2 asignacion en si corto\n");
                    if (token == corcheteF) {
                        //printf("\nCorchet F de si corto\n");
                        obtoken();
                        if (token == puntoycoma) {
                            obtoken();
                            //printf("Si corto bien\n");
                        } else {
                            test(toksig, vacio, 14);
                            //log_error(14); //faltaba punto y coma
                            sinError = 0;
                        }
                    } else {
                        //printf("Ultimo lex leido %s \n ", lex);
                        log_error(31); //faltaba corchete de cierre
                        sinError = 0;
                    }
                } else {
                    log_error(36); //se esperaba una coma
                    sinError = 0;
                }
            } else {
                if (token == corcheteF) {
                    obtoken();
                    if (token == llaveI) {
                        obtoken();
                        int sigInstruccion[NOTOKENS];
                        copia_set(sigInstruccion, toksig);
                        sigInstruccion[llaveF] = 1;
                        if (!instruccion(miArchivo, clase, metodo, sigInstruccion)) {
                            sinError = 0;
                        }
                        addLsCode(genIns(SAL, 0, 0, -1), lsCodActual);
                        struct ins* sal = NULL;
                        lastReferecence(&sal, lsCodActual);

                        if (token == llaveF) {
                            obtoken();
                            if (token == sinoTok) {
                                //printf("\n \t Sino\n");
                                obtoken();
                                if (token == llaveI) {
                                    //printf("Sino llaveI OK\n");
                                    obtoken();
                                    if (!instruccion(miArchivo, clase, metodo, sigInstruccion)) {
                                        sinError = 0;
                                    }
                                    if (token == llaveF) {
                                        obtoken();
                                        //printf("\nSino bien escrito\n");
                                    } else {
                                        test(toksig, vacio, 22);
                                        //log_error(22); //se esperaba llave de cierre
                                        sinError = 0;
                                    }
                                } else {
                                    //printf("falta llave");
                                    test(toksig, vacio, 21);
                                    //log_error(21); //se esperaba llave de apertura
                                    sinError = 0;
                                }
                            }

                          //printf("PArching\n");
                            sal->p = numeroLinea;
                            sac->p = numeroLinea;
                        } else {
                            test(toksig, vacio, 22);
                            //log_error(22); //faltaba llave de cierre
                            sinError = 0;
                        }

                    } else {
                        test(toksig, vacio, 21);
                        //log_error(21); //se esperaba llave de apertura
                        sinError = 0;
                    }
                } else {
                    test(toksig, vacio, 35);
                    //log_error(35); //se esperaba una coma o corchete de cierre
                    sinError = 0;
                }
            }
        } else {
            test(toksig, vacio, 29);
            //log_error(29); //se esperaba una corchete de apertura
            sinError = 0;
        }
    } else {
        test(toksig, vacio, 34);
        //log_error(34); // se esperaba el token si
        sinError = 0;
        //printf("Se epserab si tok");
    }
    return sinError;
}

int asignacionf(struct nodoArchivo* miArchivo, struct clase *clase, struct metodo *metodo, int toksig[]) {
    int vacio[NOTOKENS];
    init_set(vacio);
    if (token == ident) {
        //printf("\nEncontres un ident\n");

        struct atributo *atr = NULL;

        buscarAtributo(&atr, clase, lex);
        int local = 1;
        if (atr == NULL) {
            buscarAtributo2(&atr, metodo->parametros, lex);
            local = 0;
            if (atr == NULL) {
                buscarAtributo2(&atr, metodo->locales, lex);
            }
        }

        obtoken();
        if (token == asignacion) {
            //printf("\nEncontre un  = \n");
            obtoken();
            if (!expresion(miArchivo, clase, metodo, toksig)) {
                return 0;
            }
            addLsCode(genIns(ALM, local, 4, atr->atributoNum), lsCodActual);
        } else {
            test(toksig, vacio, 32);
            //log_error(32);
            return 0;
        }
    } else {
        test(toksig, vacio, 18);
        //log_error(18); //se esperaba un identificador
        return 0;
    }
    return 1;
}

int funcion_cadena(struct nodoArchivo* miArchivo, struct clase *clase, struct metodo *metodo, int toksig[]) {
    int vacio[NOTOKENS];
    int sinError = 1;
    init_set(vacio);
    if (token == concatenarTok) {
        obtoken();
        if (token == corcheteI) {
            obtoken();
            int sigValCad[NOTOKENS];
            copia_set(sigValCad, toksig);
            sigValCad[coma] = 1;
            if (!valor_cadena(miArchivo, clase, metodo, sigValCad)) {
                sinError = 0;
            }
            if (token == coma) {
                obtoken();
                sigValCad[coma] = 0;
                sigValCad[corcheteF] = 1;
                if (!valor_cadena(miArchivo, clase, metodo, sigValCad)) {
                    sinError = 0;
                }
                addLsCode(genIns(OPR, 0, 25, 0), lsCodActual);
                if (token == corcheteF) {
                    obtoken();
                    //printf("funcion concatenar completa\n");
                } else {
                    test(toksig, vacio, 31);
                    //log_error(31);
                    return 0;
                }
            } else {
                test(toksig, vacio, 36);
                //log_error(36); //se esperaba una coma
                return 0;
            }
        } else {
            test(toksig, vacio, 29);
            //log_error(29); //faltaba corchete de apertura
            return 0;
        }
    } else {
        test(toksig, vacio, 37);
        //log_error(37);
        return 0;
    }
    return sinError;
}

int funcion_archivo(struct nodoArchivo* miArchivo, struct clase *clase, struct metodo *metodo, int toksig[]) {
    int sinError = 1;
    int vacio[NOTOKENS];
    init_set(vacio);
    if (token == abrirTok) {
        obtoken();
        if (token == corcheteI) {
            obtoken();
            int sigValCad[NOTOKENS];
            copia_set(sigValCad, toksig);
            sigValCad[corcheteF] = 1;
            if (!valor_cadena(miArchivo, clase, metodo, sigValCad)) {
                sinError = 0;
            }
            if (token == corcheteF) {
                obtoken();
                //printf("funcion archivo bien escrita camino 1");
            } else {
                test(toksig, vacio, 31);
                //log_error(31); //se esperaba corchete de cierre
                return 0;
            }
        } else {
            test(toksig, vacio, 29);
            //log_error(29); //se esperaba corchete de apertura
            return 0;
        }
    } else {
        switch (token) {
            case leerLineaTok:
            case volcadoTok:
            case cerrarTok:
                obtoken();
                if (token == corcheteI) {
                    obtoken();
                    if (token == corcheteF) {
                        obtoken();
                        //printf("funcion archivo bien escrita camino 2");
                    } else {
                        test(toksig, vacio, 31);
                        //log_error(31); //se esperaba corchete de cierre
                        return 0;
                    }
                } else {
                    test(toksig, vacio, 29);
                    //log_error(29); //se esperaba corchete de abierto
                    return 0;
                }
                break;
            default:
                test(toksig, vacio, 38);
                //log_error(38); //NO deberia llegar aqui Se esperaba la llamada a funciones de archivos
                return 0;
        }
    }
    return sinError;
}

int valor_cadena(struct nodoArchivo* miArchivo, struct clase *clase, struct metodo *metodo, int toksig[]) {
    int sinError = 1;
    int vacio[NOTOKENS];
    init_set(vacio);
    switch (token) {
        case datoCadena:
            //printf("Hay una cadena\n");
            addLsCode(genInsL(LIT, 0, valorCadena, 1), lsCodActual);
            obtoken();
            break;
        case ident:
            //printf("En factor con %s\n", lex);
            if (esMetodo(clase, lex)) {
                //obtoken();
                //printf("Es un metodo\n");
                if (!llamada_metodo(miArchivo, clase, metodo, toksig)) {
                    return 0;
                }

            } else {
                struct atributo *atr = NULL;
                //printf("|||||||||||||||||EStoy buscando un objeto en la clase %s\n", clase->ident);
                int local = 0;
                int es0 = esObjeto(clase, metodo, lex, &atr, &local);
                if (es0) {
                    obtoken();
                    if (token == punto) {
                        obtoken();
                        //printf("Es una llamada a metodo o atributo\n");
                        struct clase *miClase = NULL;
                        //printf("HashClaes %d \n", atr->hashClase);
                        buscar_def_clase_hash(&miClase, atr->hashClase, &tabla); //regresa la def de la clase
                        if (miClase == NULL) {
                            test(toksig, vacio, 55);
                            return 0;
                        }
                        //printf("Ya se de que clase es \n");
                        //busca dentro de ella si es metodo o es atributo
                        //printf("------------- Es llamada a metodo \n");
                        if (esMetodo(miClase, lex)) {
                            obtoken();
                            //printf("Metodo Encontrado llamada a mi mismo metodo\n");
                            struct metodo *llamadoMetodo = NULL;
                            buscarMetodo(&llamadoMetodo, miClase, lex);

                            if (llamadoMetodo->alcance == privadoTok) {
                                if (miClase->hash != clase->hash) {
                                    log_error(61);
                                    sinError = 0;
                                }
                            }

                            if (llamada_metodo(miArchivo, clase, metodo, toksig)) {
                                sinError = 0;
                            }

                            return sinError;

                        } else {
                            //printf("------------- Es atributo \n");
                            int esatr = esAtributo(miClase, lex);
                            if (!esatr) {
                                log_error(59);
                                obtoken();
                                return 0;
                            } else {
                                struct atributo*atr = NULL;
                                buscarAtributo(&atr, miClase, lex);
                                if (atr->alcanze == privadoTok) {
                                    log_error(61);
                                    sinError = 0;
                                }

                            }
                            obtoken();
                            if (esatr == 1 && token == corcheteI) {
                                test(set_ins, toksig, 60);
                                return 0;
                            }
                            if (atr->esPorReferencia) {
                                addLsCode(genIns(CRR, local, 4, atr->atributoNum), lsCodActual);
                            } else {
                                addLsCode(genIns(CAR, local, 4, atr->atributoNum), lsCodActual);
                            }

                            return 1;

                        }
                    }

                    return 1;
                } else {

                    if (atr->esPorReferencia) {

                        addLsCode(genIns(CRR, local, 4, atr->atributoNum), lsCodActual);
                    } else {

                        addLsCode(genIns(CAR, local, 4, atr->atributoNum), lsCodActual);
                    }


                    //printf("Es una variable\n");
                    obtoken();
                    //Aqui va un car bien elaborado :)
                    //addLsCode(genIns(CAR, 0, operacion, 0), lsCodActual);


                    return 1;
                }
            }
            break;
        case concatenarTok:
            if (!funcion_cadena(miArchivo, clase, metodo, toksig)) {
                sinError = 0;
            }
            break;
        default:
            test(toksig, vacio, 15);
            //log_error(15); //se esperaba un valor cadena
            return 0;
    }

    return sinError;
}

int valor_caracter(struct nodoArchivo* miArchivo, struct clase *clase, struct metodo *metodo, int toksig[]) {
    int vacio[NOTOKENS];
    init_set(vacio);
    int sinError = 1;
    switch (token) {
        case datoCaracter:
            addLsCode(genInsL(LIT, 0, &valorCaracter, 1), lsCodActual);
            obtoken();
            break;
        case ident:
            //printf("En factor con %s\n", lex);
            if (esMetodo(clase, lex)) {
                //obtoken();
                //printf("Es un metodo\n");
                if (!llamada_metodo(miArchivo, clase, metodo, toksig)) {
                    return 0;
                }

            } else {
                struct atributo *atr = NULL;
                //printf("|||||||||||||||||EStoy buscando un objeto en la clase %s\n", clase->ident);
                int local = 0;
                int es0 = esObjeto(clase, metodo, lex, &atr, &local);
                if (es0) {
                    obtoken();
                    if (token == punto) {
                        obtoken();
                        //printf("Es una llamada a metodo o atributo\n");
                        struct clase *miClase = NULL;
                        //printf("HashClaes %d \n", atr->hashClase);
                        buscar_def_clase_hash(&miClase, atr->hashClase, &tabla); //regresa la def de la clase
                        if (miClase == NULL) {
                            test(toksig, vacio, 55);
                            return 0;
                        }
                        //printf("Ya se de que clase es \n");
                        //busca dentro de ella si es metodo o es atributo
                        //printf("------------- Es llamada a metodo \n");
                        if (esMetodo(miClase, lex)) {
                            obtoken();
                            //printf("Metodo Encontrado llamada a mi mismo metodo\n");
                            struct metodo *llamadoMetodo = NULL;
                            buscarMetodo(&llamadoMetodo, miClase, lex);

                            if (llamadoMetodo->alcance == privadoTok) {
                                if (miClase->hash != clase->hash) {
                                    log_error(61);
                                    sinError = 0;
                                }
                            }

                            if (llamada_metodo(miArchivo, clase, metodo, toksig)) {
                                sinError = 0;
                            }

                            return sinError;

                        } else {
                            //printf("------------- Es atributo \n");
                            int esatr = esAtributo(miClase, lex);
                            if (!esatr) {
                                log_error(59);
                                obtoken();
                                return 0;
                            } else {
                                struct atributo*atr = NULL;
                                buscarAtributo(&atr, miClase, lex);
                                if (atr->alcanze == privadoTok) {
                                    log_error(61);
                                    sinError = 0;
                                }

                            }
                            obtoken();
                            if (esatr == 1 && token == corcheteI) {
                                test(set_ins, toksig, 60);
                                return 0;
                            }
                            if (atr->esPorReferencia) {
                                addLsCode(genIns(CRR, local, 4, atr->atributoNum), lsCodActual);
                            } else {
                                addLsCode(genIns(CAR, local, 4, atr->atributoNum), lsCodActual);
                            }

                            return 1;

                        }
                    }

                    return 1;
                } else {

                    if (atr->esPorReferencia) {

                        addLsCode(genIns(CRR, local, 4, atr->atributoNum), lsCodActual);
                    } else {

                        addLsCode(genIns(CAR, local, 4, atr->atributoNum), lsCodActual);
                    }


                    //printf("Es una variable\n");
                    obtoken();
                    //Aqui va un car bien elaborado :)
                    //addLsCode(genIns(CAR, 0, operacion, 0), lsCodActual);


                    return 1;
                }
            }
            break;
        default:
            test(toksig, vacio, 15);
            //log_error(15);
            return 0;
    }
    return 1;
}

int llamada_metodo(struct nodoArchivo* miArchivo, struct clase *clase, struct metodo *metodo, int toksig[]) {
    int sinError = 1;
    int vacio[NOTOKENS];
    init_set(vacio);
    if (token == ident) {
        obtoken();
        if (token == corcheteI) {
            obtoken();
            //Vefiricar la cantidad de parametro de este metodo
            if (token == corcheteF) {
                //printf("BUena llamada a metodo\n");
                obtoken();
            } else {
                int sigExpresion[NOTOKENS];
                copia_set(sigExpresion, toksig);
                sigExpresion[coma] = sigExpresion[corcheteF] = 1;
                if (token == referencia) {
                    obtoken();
                    if (token == ident) {
                        obtoken();
                        //printf("BUena llamada a metodo 2\n");
                    } else {
                        test(toksig, sigExpresion, 18);
                        //log_error(18); //se espera un ident
                        sinError = 0;
                    }
                }
                if (!expresion(miArchivo, clase, metodo, sigExpresion)) {
                    sinError = 0;
                }

                int i = 2;
                while (token == coma) {
                    //printf("Parametro %d\n", i);
                    obtoken();
                    if (token == referencia) {
                        obtoken();
                        if (token == ident) {
                            obtoken();
                        } else {
                            test(toksig, sigExpresion, 18);
                            //log_error(18); //se espera un ident
                            sinError = 0;
                        }
                    }
                    if (!expresion(miArchivo, clase, metodo, sigExpresion)) {
                        sinError = 0;
                    }

                    i++;
                }
                if (token == corcheteF) {
                    //printf("BUena llamada a metodo\n");
                    obtoken();
                } else {
                    test(toksig, vacio, 31);
                    //log_error(31); //Falta corchete F
                    return 0;
                }
            }

        } else {
            test(toksig, vacio, 29);
            //log_error(29); //falta corchete de apertura
            return 0;
        }
    } else {
        if (!funcion_arreglo(miArchivo, clase, metodo, toksig)) {
            return 0;
        }
    }
    return sinError;
}

int expresion(struct nodoArchivo* miArchivo, struct clase *clase, struct metodo *metodo, int toksig[]) {
    int vacio[NOTOKENS];
    init_set(vacio);
    if (token == verdaderoTok || token == falsoTok) {
        //printf("Era un booleano \n");
        int a = (token == verdaderoTok) ? 1 : 0;
        addLsCode(genInsL(LIT, 0, &a, 6), lsCodActual);
        obtoken();
        return 1;
    }
    if (token == datoCadena) {
        //printf("se encontro en la expresión una cadena\n");
        addLsCode(genInsL(LIT, 0, valorCadena, 1), lsCodActual);
        obtoken();
        return 1;
    }
    if (token == concatenarTok) {
        if (!funcion_cadena(miArchivo, clase, metodo, toksig)) {
            return 0;
        }
        addLsCode(genIns(OPR, 0, 2, 0), lsCodActual);
        //printf("se encontro una funcion concatenar\n");
        return 1;
    }
    if (token == datoCaracter) {
        //printf("Encontramos un caracter\n");
        addLsCode(genInsL(LIT, 0, &valorCaracter, 2), lsCodActual);
        obtoken();
        return 1;
    }
    if (expresion_numerica(miArchivo, clase, metodo, toksig) == 1) {
        return 1;
    }
    test(toksig, vacio, 57);
    return 0;
}

int expresion_numerica(struct nodoArchivo* miArchivo, struct clase *clase, struct metodo *metodo, int toksig[]) {
    int sinError = 1;
    int sigExpCon[NOTOKENS];
    copia_set(sigExpCon, toksig);
    sigExpCon[otok] = 1;
    if (!expresion_conjuncion(miArchivo, clase, metodo, sigExpCon)) {
        sinError = 0;
    }
    while (token == otok) {
        obtoken();
        //printf("En expresion numerica con %s \n", lex);
        if (!expresion_conjuncion(miArchivo, clase, metodo, sigExpCon)) {
            sinError = 0;
        }
        addLsCode(genIns(OPR, 0, 13, 0), lsCodActual);
    }
    return sinError;
}

int expresion_conjuncion(struct nodoArchivo* miArchivo, struct clase *clase, struct metodo *metodo, int toksig[]) {
    int sinError = 1;
    int sigExpRel[NOTOKENS];
    copia_set(sigExpRel, toksig);
    sigExpRel[ytok] = 1;
    if (!expresion_relacional(miArchivo, clase, metodo, sigExpRel)) {
        sinError = 0;
    }
    while (token == ytok) {
        obtoken();
        //printf("En expresion conjuncion con %s\n", lex);
        if (!expresion_relacional(miArchivo, clase, metodo, sigExpRel)) {
            sinError = 0;
        }
        addLsCode(genIns(OPR, 0, 14, 0), lsCodActual);
    }
    return sinError;
}

int expresion_relacional(struct nodoArchivo* miArchivo, struct clase *clase, struct metodo *metodo, int toksig[]) {
    int sinError = 1;

    int sigExpAri[NOTOKENS];
    copia_set(sigExpAri, toksig);
    sigExpAri[mnr] = sigExpAri[myr] = sigExpAri[mai] = sigExpAri[mei] = 1;
    sigExpAri[igl] = sigExpAri[nig] = 1;
    int negar = 0;
    if (token == negacion) {
        //printf("se encontro negación\n");
        negar = 1;
        obtoken();
    }

    if (!expresion_aritmetrica(miArchivo, clase, metodo, sigExpAri)) {
        sinError = 0;
    }
    if (token == mnr || token == myr || token == mai || token == mei || token == igl || token == nig) {

        int operacion;
        switch (token) {
            case mnr: operacion = 6;
                break;
            case myr: operacion = 7;
                break;
            case mai: operacion = 8;
                break;
            case mei: operacion = 9;
                break;
            case igl: operacion = 10;
                break;
            case nig: operacion = 11;
                break;
        }
        obtoken();
        //printf("Exapresion relacional con %s\n", lex);
        if (!expresion_aritmetrica(miArchivo, clase, metodo, sigExpAri)) {
            sinError = 0;
        }
        // printf("2 Ponioen %d\n ", operacion);
        addLsCode(genIns(OPR, 0, operacion, 0), lsCodActual);

    }

    if (negar) {
        addLsCode(genIns(OPR, 0, 12, 0), lsCodActual);
    }
    return sinError;
}

int expresion_aritmetrica(struct nodoArchivo* miArchivo, struct clase *clase, struct metodo *metodo, int toksig[]) {
    int sinError = 1;
    int sigTermino[NOTOKENS];
    copia_set(sigTermino, toksig);
    sigTermino[mas] = sigTermino[menos] = 1;
    if (!termino(miArchivo, clase, metodo, sigTermino)) {
        sinError = 0;
    }

    if (token == mas || token == menos) {
        do {
            int operacion = (token == mas) ? 2 : 3;
            obtoken();
            //printf("Exapresion aritmetica con %s\n", lex);
            if (!termino(miArchivo, clase, metodo, sigTermino)) {
                sinError = 0;
            }
            //printf("3 Ponioen %d\n ", operacion);
            addLsCode(genIns(OPR, 0, operacion, 0), lsCodActual);
        } while (token == mas || token == menos);
    }
    return sinError;
}

int termino(struct nodoArchivo* miArchivo, struct clase *clase, struct metodo *metodo, int toksig[]) {
    int sinError = 1;
    int sigFactor[NOTOKENS];
    copia_set(sigFactor, toksig);
    sigFactor[por] = sigFactor[barra] = 1;
    if (!factor(miArchivo, clase, metodo, sigFactor)) {
        sinError = 0;
    }

    if (token == por || token == barra) {
        do {
            int operacion = (token == por) ? 4 : 5;

            obtoken();
            if (!factor(miArchivo, clase, metodo, sigFactor)) {
                sinError = 0;
            }
            //printf("4 Ponioen %d\n ", operacion);
            addLsCode(genIns(OPR, 0, operacion, 0), lsCodActual);

        } while (token == por || token == barra);
    }
    return sinError;
    ////printf("\nTerminio termino");
}

/* Para escribir todo los cambios de signo*/

void cambioSigno(int n) {
    int i = 1;
    for (i = 1; i <= n; i++) {
        addLsCode(genIns(OPR, 0, 1, 0), lsCodActual);
    }
}

int factor(struct nodoArchivo* miArchivo, struct clase *clase, struct metodo *metodo, int toksig[]) {
    int sinError = 1;
    int vacio[NOTOKENS];
    init_set(vacio);
    //printf("En factor ---\n");

    int mNos = 0;
    while (token == mas || token == menos) {
        if (token == menos) {
            mNos++;
        }
        obtoken();
    }

    if (token == numeroEntero || token == numeroReal) {

        addLsCode(genInsL(LIT, 0, (token == numeroEntero) ? (void *) &valor : (void *) &valorDoble, (token == numeroEntero) ? 3 : 4), lsCodActual);
        obtoken();
        //printf("\n\tEncontre un numero\n");
        cambioSigno(mNos);
        return 1;
    }
    if (token == parentI) {
        obtoken();
        int sigExpNum[NOTOKENS];
        copia_set(sigExpNum, toksig);
        sigExpNum[parentF] = 1;
        if (!expresion_numerica(miArchivo, clase, metodo, sigExpNum)) {
            sinError = 0;
        }
        cambioSigno(mNos);
        if (token == parentF) {
            obtoken();
            ////printf("Se encontro una expresion numerica con parentesis\n");
            return sinError;
        } else {
            test(toksig, vacio, 42);
            //log_error(42); //se esperaba un token de cierre
            return 0;
        }
    }

    if (token == sistemaTok) {
        //obtoken();
        if (!funcion_num_numcad(miArchivo, clase, metodo, toksig)) {
            return 0;
        }
        cambioSigno(mNos);
        return 1;
    }
    if (token == ident) {
        //printf("En factor con %s\n", lex);
        if (esMetodo(clase, lex)) {
            //obtoken();
            //printf("Es un metodo\n");
            if (!llamada_metodo(miArchivo, clase, metodo, toksig)) {
                return 0;
            }
            cambioSigno(mNos);
        } else {
            struct atributo *atr = NULL;
            //printf("|||||||||||||||||EStoy buscando un objeto en la clase %s\n", clase->ident);
            int local = 0;
            int es0 = esObjeto(clase, metodo, lex, &atr, &local);
            if (es0) {
                obtoken();
                if (token == punto) {
                    obtoken();
                    //printf("Es una llamada a metodo o atributo\n");
                    struct clase *miClase = NULL;
                    //printf("HashClaes %d \n", atr->hashClase);
                    buscar_def_clase_hash(&miClase, atr->hashClase, &tabla); //regresa la def de la clase
                    if (miClase == NULL) {
                        test(toksig, vacio, 55);
                        return 0;
                    }
                    //printf("Ya se de que clase es \n");
                    //busca dentro de ella si es metodo o es atributo
                    //printf("------------- Es llamada a metodo \n");
                    if (esMetodo(miClase, lex)) {
                        obtoken();
                        //printf("Metodo Encontrado llamada a mi mismo metodo\n");
                        struct metodo *llamadoMetodo = NULL;
                        buscarMetodo(&llamadoMetodo, miClase, lex);

                        if (llamadoMetodo->alcance == privadoTok) {
                            if (miClase->hash != clase->hash) {
                                log_error(61);
                                sinError = 0;
                            }
                        }

                        if (llamada_metodo(miArchivo, clase, metodo, toksig)) {
                            sinError = 0;
                        }
                        cambioSigno(mNos);
                        return sinError;

                    } else {
                        //printf("------------- Es atributo \n");
                        int esatr = esAtributo(miClase, lex);
                        if (!esatr) {
                            log_error(59);
                            obtoken();
                            return 0;
                        } else {
                            struct atributo*atr = NULL;
                            buscarAtributo(&atr, miClase, lex);
                            if (atr->alcanze == privadoTok) {
                                log_error(61);
                                sinError = 0;
                            }

                        }
                        obtoken();
                        if (esatr == 1 && token == corcheteI) {
                            test(set_ins, toksig, 60);
                            return 0;
                        }
                        if (atr->esPorReferencia) {
                            addLsCode(genIns(CRR, local, 4, atr->atributoNum), lsCodActual);
                        } else {
                            addLsCode(genIns(CAR, local, 4, atr->atributoNum), lsCodActual);
                        }
                        cambioSigno(mNos);
                        return 1;

                    }
                }
                if (atr->esPorReferencia) {
                    addLsCode(genIns(CRR, local, 4, atr->atributoNum), lsCodActual);
                } else {
                    addLsCode(genIns(CAR, local, 4, atr->atributoNum), lsCodActual);
                }

                cambioSigno(mNos);
                return 1;
            } else {

                if (atr->esPorReferencia) {

                    addLsCode(genIns(CRR, local, 4, atr->atributoNum), lsCodActual);
                } else {

                    addLsCode(genIns(CAR, local, 4, atr->atributoNum), lsCodActual);
                }

                cambioSigno(mNos);
                //printf("Es una variable\n");
                obtoken();
                //Aqui va un car bien elaborado :)
                //addLsCode(genIns(CAR, 0, operacion, 0), lsCodActual);


                return 1;
            }
        }
    }




    //printf("Sin factor\n");
    test(toksig, vacio, 43);
    //log_error(43); //se esperaba un factor
    return 0;
}

int funcion_num_numcad(struct nodoArchivo* miArchivo, struct clase *clase, struct metodo *metodo, int toksig[]) {
    int sinError = 1;
    int vacio[NOTOKENS];
    init_set(vacio);
    int op1 = 14, op2 = 18;

  //printf("Entrando en funcion numerica \n ");
    if (token == sistemaTok) {
        obtoken();
        if (token == punto) {
            obtoken();
            switch (token) {
                case parteEnteraTok:
                    op1++;
                case esParTok:
                    op1++;
                case decimalBinTok:
                    op1++;
                case absolutoTok:
                    op1++;
                    obtoken();
                    if (token == corcheteI) {
                        obtoken();
                        int sigExpNum[NOTOKENS];
                        copia_set(sigExpNum, toksig);
                        sigExpNum[corcheteF] = 1;
                        if (!expresion_numerica(miArchivo, clase, metodo, sigExpNum)) {
                            sinError = 0;
                        }
                      //printf("5 Ponioen %d\n ", op1);
                        addLsCode(genIns(OPR, 0, op1, 0), lsCodActual);
                        if (token == corcheteF) {
                            obtoken();
                        } else {
                            test(toksig, vacio, 31);
                            //log_error(31); //Se esperaba corchete de cierre
                            return 0;
                        }
                    } else {
                        test(toksig, vacio, 29);
                        //log_error(29); //Se esperaba corchet apertura
                        return 0;
                    }
                    break;
                case mayorTok:
                    op2++;
                case menorTok:
                    op2++;
                case potenciaTok:
                    op2++;
                case moduloTok:
                    op2++;
                    //printf("Funcion prederteinada\n");
                    obtoken();
                    if (token == corcheteI) {
                        obtoken();
                        int sigExpNum[NOTOKENS];
                        copia_set(sigExpNum, toksig);
                        sigExpNum[coma] = 1;
                        if (!expresion_numerica(miArchivo, clase, metodo, sigExpNum)) {
                            sinError = 0;
                        }
                        if (token == coma) {
                            obtoken();
                            sigExpNum[coma] = 0;
                            sigExpNum[corcheteF] = 1;
                            if (!expresion_numerica(miArchivo, clase, metodo, sigExpNum)) {
                                sinError = 0;
                            }
                          //printf("6 Ponioen %d\n ", op2);
                            addLsCode(genIns(OPR, 0, op2, 0), lsCodActual);
                            if (token == corcheteF) {
                                obtoken();
                            } else {
                                test(toksig, vacio, 31);
                                //log_error(31); //Se esperaba corchete de cierre
                                return 0;
                            }
                        } else {
                            test(toksig, vacio, 36);
                            //log_error(36); //falto coma
                            return 0;
                        }
                    } else {
                        test(toksig, vacio, 29);
                        //log_error(29); //Se esperaba corchet apertura
                        return 0;
                    }
                    break;
                case longitudCadenaTok:
                    obtoken();
                    if (token == corcheteI) {
                        obtoken();
                        int sigValCad[NOTOKENS];
                        copia_set(sigValCad, toksig);
                        sigValCad[corcheteF] = 1;
                        if (!valor_cadena(miArchivo, clase, metodo, sigValCad)) {
                            sinError = 0;
                        }
                        addLsCode(genIns(OPR, 0, 23, 0), lsCodActual);
                        if (token == corcheteF) {
                            obtoken();
                        } else {
                            test(toksig, vacio, 31);
                            //log_error(31); //Se esperaba corchete de cierre
                            return 0;
                        }
                    } else {
                        test(toksig, vacio, 29);
                        //log_error(29); //Se esperaba corchet apertura
                        return 0;
                    }
                    break;
                case compararTok:
                    obtoken();
                    if (token == corcheteI) {
                        obtoken();
                        int sigValCad[NOTOKENS];
                        copia_set(sigValCad, toksig);
                        sigValCad[coma] = 1;
                        if (!valor_cadena(miArchivo, clase, metodo, sigValCad)) {
                            sinError = 0;
                        }
                        if (token == coma) {
                            obtoken();
                            sigValCad[coma] = 0;
                            sigValCad[corcheteF] = 1;
                            if (!valor_cadena(miArchivo, clase, metodo, sigValCad)) {
                                sinError = 0;
                            }
                            addLsCode(genIns(OPR, 0, 24, 0), lsCodActual);
                            if (token == corcheteF) {
                                obtoken();
                            } else {
                                test(toksig, vacio, 31);
                                //log_error(31); //Se esperaba corchete de cierre
                                return 0;
                            }
                        } else {
                            test(toksig, vacio, 36);
                            //log_error(36); //falto coma
                            return 0;
                        }
                    } else {
                        test(toksig, vacio, 29);
                        //log_error(29); //Se esperaba corchet apertura
                        return 0;
                    }
                    break;
                default:
                    test(toksig, vacio, 44);
                    //log_error(44); // se esperaba llamada a sistema;
                    return 0;
                    break;
            }
        } else {
            test(toksig, vacio, 45);
            //log_error(45); // se esperaba un punto
            return 0;
        }
    } else {
        test(toksig, vacio, 46);
        //log_error(46); //Se esperaba palabra reservada sistema
        return 0;
    }
    return sinError;
}

int instruccion_mientras(struct nodoArchivo* miArchivo, struct clase *clase, struct metodo *metodo, int toksig[]) {
    int vacio[NOTOKENS];
    init_set(vacio);
    int sinError = 1;
    if (token == mientrasTok) {
        obtoken();
        if (token == corcheteI) {
            obtoken();
            int sigExpesion[NOTOKENS];
            copia_set(sigExpesion, toksig);
            sigExpesion[corcheteF] = 1;
            int inicioExpresion = numeroLinea;
            if (!expresion(miArchivo, clase, metodo, sigExpesion)) {
                sinError = 0;
            }


            addLsCode(genIns(SAC, 0, 0, -1), lsCodActual);
            struct ins* sac = NULL;
            lastReferecence(&sac, lsCodActual);

            if (token == corcheteF) {
                obtoken();
                if (token == llaveI) {
                    obtoken();
                    int sigInstruccion[NOTOKENS];
                    copia_set(sigInstruccion, toksig);
                    sigInstruccion[llaveF] = 1;
                    if (!instruccion(miArchivo, clase, metodo, sigInstruccion)) {
                        sinError = 0;
                    }

                    addLsCode(genIns(SAL, 0, 0, inicioExpresion), lsCodActual);
                    sac->p = numeroLinea;

                    if (token == llaveF)
                        obtoken();
                    else {
                        test(toksig, vacio, 22);
                        //log_error(22); //no esta }
                        return 0;
                    }
                } else {
                    test(toksig, vacio, 21);
                    //log_error(21); //no esta {
                    return 0;
                }
            } else {
                test(toksig, vacio, 31);
                //log_error(31); //no esta el ]
                return 0;
            }
        } else {
            test(toksig, vacio, 29);
            //log_error(29); //no esta el [
            return 0;
        }
    } else {
        test(toksig, vacio, 47);
        //log_error(47); // no es el token mientras
        return 0;
    }
    return sinError;
}

int instruccion_romper(int toksig[]) {
    int vacio[NOTOKENS];
    init_set(vacio);
    if (token == romperTok) {
        obtoken();
        // addLsCode(genIns(SAL, 0, 0, -100000000), lsCodActual);
        if (token == puntoycoma)
            obtoken();
        else {
            test(toksig, vacio, 14);
            //log_error(14); //no esta el ;
            return 0;
        }
    } else {
        test(toksig, vacio, 48);
        //log_error(48); // no esta el token romper
        return 0;
    }
    return 1;
}

int funcion_arreglo(struct nodoArchivo* miArchivo, struct clase *clase, struct metodo *metodo, int toksig[]) {
    int sinError = 1;
    int vacio[NOTOKENS];
    init_set(vacio);
    switch (token) {
        case agregarTok:
        case obtenerTok:
        case quitarTok:
            obtoken();
            if (token == corcheteI) {
                obtoken();
                int sigExpresion[NOTOKENS];
                copia_set(sigExpresion, toksig);
                sigExpresion[corcheteF] = 1;
                if (!expresion(miArchivo, clase, metodo, sigExpresion)) {
                    sinError = 0;
                }
                if (token == corcheteF) {
                    obtoken();
                    //printf("funcion arreglo ok\n");
                } else {
                    test(toksig, vacio, 31);
                    //log_error(31); //se esperaba corchete de cierre
                    return 0;
                }
            } else {
                test(toksig, vacio, 29);
                //log_error(29); //se esperaba corchete de apertura
                return 0;
            }
            break;
        case cuantosTok:
            obtoken();
            if (token == corcheteI) {
                obtoken();
                if (token == corcheteF) {
                    obtoken();
                    ////printf("funcion cuantos ok\n");
                    return 1;
                } else {
                    test(toksig, vacio, 31);
                    //log_error(31); //se esperaba corchete de cierre
                    return 0;
                }
            } else {
                test(toksig, vacio, 29);
                //log_error(29); //se esperaba corchete de apertura
                return 0;
            }
            break;
        default:
            test(toksig, vacio, 49);
            //log_error(49); //no es una funcion arreglo
            return 0;
    }
    return sinError;
}

int instruccion_es(struct nodoArchivo* miArchivo, struct clase *clase, struct metodo *metodo, int toksig[]) {
    int sinError = 1;
    int vacio[NOTOKENS];
    init_set(vacio);

    int op = 28;

    if (token == sistemaTok) {
        obtoken();
        if (token == punto) {
            obtoken();
            switch (token) {
                case obtenerEnteroTok:
                    op++;
                case obtenerRealTok:
                    op++;
                case obtenerCadenaTok:
                    op++;
                case obtenerCaracterTok:
                    op++;
                    obtoken();
                    if (token == corcheteI) {
                        //printf("1 Ponioen %d\n ", op);
                        addLsCode(genIns(OPR, 0, op, 0), lsCodActual);
                        obtoken();
                        if (token == ident) {

                            struct atributo *atr = NULL;
                            int local;
                            int es0 = esObjeto(clase, metodo, lex, &atr, &local);

                            if (atr == NULL) {
                                log_error(23);
                            } else {
                                if (atr->esPorReferencia) {
                                    addLsCode(genIns(CRR, local, 4, atr->atributoNum), lsCodActual);
                                } else {
                                    addLsCode(genIns(CAR, local, 4, atr->atributoNum), lsCodActual);
                                }
                            }
                            obtoken();


                            if (token == corcheteF) {
                                obtoken();
                                if (token == puntoycoma) {
                                    obtoken();
                                } else {
                                    test(toksig, vacio, 14);
                                    //log_error(14); //Se esperaba punto y coma
                                    return 0;
                                }
                            } else {
                                test(toksig, vacio, 31);
                                //log_error(31); // se esperaba corchete de cierre
                                return 0;
                            }
                        } else {
                            test(toksig, vacio, 18);
                            //log_error(18); //se esperaba indentificador
                            return 0;
                        }
                    } else {
                        test(toksig, vacio, 29);
                        //log_error(29); // se esperaba corchete de apertura
                        return 0;
                    }
                    break;
                case mostrarTok:
                    obtoken();
                    if (token == corcheteI) {
                        obtoken();
                        int sigExpresion[NOTOKENS];
                        copia_set(sigExpresion, toksig);
                        sigExpresion[coma] = sigExpresion[corcheteF] = 1;
                        if (!expresion(miArchivo, clase, metodo, sigExpresion)) {
                            sinError = 0;
                        }

                        addLsCode(genIns(OPR, 0, 30, 0), lsCodActual);

                        /*if (token == coma) {
                            obtoken();
                            sigExpresion[coma] = 0;
                            if (!expresion(miArchivo, clase, metodo, sigExpresion)) {
                                sinError = 0;
                            }
                        }*/
                        if (token == corcheteF) {
                            obtoken();
                            if (token == puntoycoma) {
                                obtoken();
                            } else {
                                test(toksig, vacio, 4);
                                //log_error(14); //Se esperaba punto y coma
                                return 0;
                            }
                        } else {
                            test(toksig, vacio, 2);
                            //log_error(2); // se esperaba corchete de cierre
                            return 0;
                        }
                    } else {
                        test(toksig, vacio, 3);
                        //log_error(3); // se esperaba corchet ede apartura
                        return 0;
                    }
                    break;
                default:
                    test(toksig, vacio, 4);
                    //log_error(4); //se esperaba un token de ontener o mostrar
                    return 0;
            }
        } else {
            test(toksig, vacio, 5);
            //log_error(5); // se esperaba un punto
            return 0;
        }
    } else {

        test(toksig, vacio, 6);
        //log_error(6); //se esperaba el token sistema
        return 0;
    }
    return sinError;
}

int instruccion_probar(struct nodoArchivo* miArchivo, struct clase *clase, struct metodo *metodo, int toksig[]) {
    int vacio[NOTOKENS];
    int sinError = 1;
    init_set(vacio);
    if (token == probarTok) {
        obtoken();
        if (token == corcheteI) {
          //printf("Probar \n");
            obtoken();
            int sigExpresion[NOTOKENS];
            copia_set(sigExpresion, toksig);
            sigExpresion[corcheteF] = 1;
            struct lsCod expresionc;

            struct lsCod *lsCodActualCopia = lsCodActual;
            lsCodActual = &expresionc;
            expresionc.sig = NULL;
            expresionc.instruccion.f = INS;
            expresionc.instruccion.d = -6;
            expresionc.instruccion.n = -6;
            expresionc.instruccion.p = -6;


            if (!expresion(miArchivo, clase, metodo, sigExpresion)) {
                sinError = 0;
            }

            lsCodActual = lsCodActualCopia;
            //addBlockLs(expresionc,lsCodActual);
            //addLsCode(genIns(OPR, 0, 10, 0), lsCodActual);
            //addLsCode(genIns(SAC, 0, 0, -1), lsCodActual);
            // struct ins* sac0 = NULL;
            // lastReferecence(&sac0, lsCodActual);





            if (token == corcheteF) {
                obtoken();
                if (token == llaveI) {
                    obtoken();
                    if (token == casoTok) {
                        struct ins * sac[100];
                        struct ins * sal[100];
                        int posS = 0, posSal = 0;
                        while (token == casoTok) {


                            obtoken();
                            sigExpresion[corcheteF] = 0;
                            sigExpresion[dosPuntos] = 1;
                            int linea = numeroLinea;
                          //printf("Caso \n");
                            if (!expresion(miArchivo, clase, metodo, sigExpresion)) {
                                sinError = 0;
                            }

                            addBlockLs(expresionc, lsCodActual);
                            addLsCode(genIns(OPR, 0, 10, 0), lsCodActual);

                            addLsCode(genIns(SAC, 0, 0, -1), lsCodActual);
                            sac[posS] = NULL;
                            lastReferecence(&(sac[posS]), lsCodActual);

                            if (posS != 0) {
                                sac[posS - 1]->p = linea;
                            }
                            posS++;

                            int sigIns[NOTOKENS];
                            copia_set(sigIns, toksig);
                            sigIns[casoTok] = sigIns[defectoTok] = sigIns[llaveF] = 1;
                            if (token == dosPuntos) {
                                obtoken();

                                if (!instruccion(miArchivo, clase, metodo, sigIns)) {
                                    sinError = 0;
                                }
                              //printf("Retornando a caso\n");
                                addLsCode(genIns(SAL, 0, 0, -1), lsCodActual);

                                sal[posSal] = NULL;
                                lastReferecence(&(sal[posSal]), lsCodActual);
                                posSal++;

                            } else {
                                test(toksig, sigIns, 50);
                                // log_error(50); //faltan :
                                sinError = 0;
                            }
                        }

                        sac[posS - 1]->p = numeroLinea;

                        if (token == defectoTok) {
                            obtoken();
                            int sig[NOTOKENS];
                            init_set(sig);
                            sig[llaveF] = 1;
                            if (token == dosPuntos) {
                                obtoken();
                                copia_set(sig, toksig);
                                sac[posS - 1]->p = numeroLinea + 1;
                                if (!instruccion(miArchivo, clase, metodo, sig)) {
                                    sinError = 0;
                                }
                            } else {

                                test(toksig, sig, 50);
                                //log_error(50); //falta :
                                sinError = 0;
                            }
                        }
                        int i = 0;
                        for (i = 0; i < posSal; i++) {
                            sal[i]->p = numeroLinea + 1;
                        }
                        if (token == llaveF) {
                            obtoken();
                        } else {
                            test(toksig, vacio, 22);
                            // log_error(22); //falta }
                            return 0;
                        }
                    } else {
                        // log_error(51); //falta caso
                        test(toksig, vacio, 51);
                        return 0;
                    }
                } else {
                    test(toksig, vacio, 21);
                    //log_error(21); //falta {
                    return 0;
                }
            } else {
                test(toksig, vacio, 31);
                //log_error(31); //falta ]
                return 0;
            }
        } else {
            test(toksig, vacio, 29);
            //log_error(29); //falta [
            return 0;
        }
    } else {
        test(toksig, vacio, 52);

        //log_error(52); //falta probar
        return 0;
    }
    return sinError;
}

int instruccion_para(struct nodoArchivo* miArchivo, struct clase *clase, struct metodo *metodo, int toksig[]) {
    int sinError = 1;
    int vacio[NOTOKENS];
    init_set(vacio);

    if (token == paraTok) {
        obtoken();
        if (token == corcheteI) {
            obtoken();
            if (token == ident) {
                //verificar que exista en este metodo o clase
                struct atributo *atr = NULL;

                buscarAtributo(&atr, clase, lex);
                int local = 1;
                if (atr == NULL) {
                    buscarAtributo2(&atr, metodo->parametros, lex);
                    local = 0;
                    if (atr == NULL) {
                        buscarAtributo2(&atr, metodo->locales, lex);
                    }
                }

                if (atr == NULL) {
                    log_error(23);
                } else {
                    if (!atr->esPrimitivo || atr->tipo != ENTERO) {
                        log_error(63);
                        atr = NULL;
                    }
                }
                obtoken();
                if (token == asignacion) {
                    obtoken();
                    int sigExp[NOTOKENS];
                    copia_set(sigExp, toksig);
                    sigExp[coma] = 1;
                  //printf("Varible para\n");
                    if (!expresion_numerica(miArchivo, clase, metodo, sigExp)) {
                        sinError = 0;
                    }
                    if (atr != NULL) {
                        addLsCode(genIns(ALM, local, 4, atr->atributoNum), lsCodActual);
                        addLsCode(genIns(CAR, local, 4, atr->atributoNum), lsCodActual);
                    }
                    if (token == coma) {
                        obtoken();
                      //printf("COndicion para\n");
                        int linea = numeroLinea;
                        if (!expresion_numerica(miArchivo, clase, metodo, sigExp)) {
                            sinError = 0;
                        }

                        addLsCode(genIns(SAC, 0, 0, -1), lsCodActual);
                        struct ins *sac = NULL;
                        lastReferecence(&sac, lsCodActual);

                        if (token == coma) {
                            obtoken();
                            sigExp[coma] = 0;
                            sigExp[corcheteF] = 1;

                            struct lsCod *tmp = lsCodActual;
                            struct lsCod actulizarD;
                            actulizarD.sig = NULL;

                            actulizarD.instruccion.p = -6;
                            lsCodActual = &actulizarD;
                          //printf("Aumento para\n");
                            if (!asignacionf(miArchivo, clase, metodo, sigExp)) {
                                sinError = 0;
                            }
                            if (token == corcheteF) {
                                obtoken();
                                if (token == puntoycoma) {
                                    obtoken();
                                } else {
                                    if (token == llaveI) {
                                        obtoken();
                                        sigExp[coma] = 0;
                                        sigExp[corcheteF] = 0;
                                        sigExp[llaveF] = 0;

                                        lsCodActual = tmp;
                                      //printf("Instrucciones para\n");
                                        if (!instruccion(miArchivo, clase, metodo, sigExp)) {
                                            sinError = 0;
                                        }
                                        addBlockLs(actulizarD, lsCodActual);
                                        addLsCode(genIns(SAL, 0, 0, linea), lsCodActual);

                                        sac->p = numeroLinea;

                                        if (token == llaveF) {
                                            obtoken();
                                        } else {
                                            //log_error(22); //falta }
                                            test(toksig, vacio, 22);
                                            return 0;
                                        }
                                    } else {
                                        //log_error(14); // falta el ; 
                                        test(toksig, vacio, 14);
                                        return 0;
                                    }
                                }
                            } else {
                                //log_error(31); //falta ]
                                test(toksig, vacio, 31);
                                return 0;
                            }
                        } else {
                            test(toksig, vacio, 36);
                            //log_error(36); //falta ,
                            return 0;
                        }
                    } else {
                        test(toksig, vacio, 36);
                        //log_error(36); //falta ,
                        return 0;
                    }
                } else {
                    test(toksig, vacio, 32);
                    //log_error(32); //falta =
                    return 0;
                }
            } else {
                test(toksig, vacio, 18);
                //log_error(18); //no es un identificador
                return 0;
            }
        } else {
            test(toksig, vacio, 29);
            //log_error(29); //no es [
            return 0;
        }
    } else {
        test(toksig, vacio, 53);
        //log_error(53); //no es la palabra para
        return 0;
    }
    return sinError;
}

int instruccion_hacer(struct nodoArchivo* miArchivo, struct clase *clase, struct metodo *metodo, int toksig[]) {
    int sinError = 1;
    int vacio[NOTOKENS];
    init_set(vacio);
    if (token == hacerTok) {
        obtoken();
        if (token == llaveI) {
            obtoken();
            int sigIns[NOTOKENS];
            copia_set(sigIns, toksig);
            sigIns[llaveF] = 1;
            int linea  = numeroLinea;
            
            if (!instruccion(miArchivo, clase, metodo, sigIns)) {
                sinError = 0;
            }
            if (token == llaveF) {
                obtoken();
                if (token == mientrasTok) {
                    obtoken();
                    if (token == corcheteI) {
                        obtoken();
                        int sigExp[NOTOKENS];
                        copia_set(sigExp, toksig);
                        sigExp[corcheteF] = 1;
                        if (!expresion_numerica(miArchivo, clase, metodo, sigExp)) {
                            sinError = 0;
                        }
                        addLsCode(genIns(SAC,0,0,numeroLinea + 2),lsCodActual);
                        addLsCode(genIns(SAL,0,0,linea), lsCodActual);
                        if (token == corcheteF) {
                            obtoken();
                            if (token == puntoycoma)
                                obtoken();
                            else {
                                test(toksig, vacio, 14);
                                //log_error(14); //falta ;
                                return 0;
                            }
                        } else {
                            test(toksig, vacio, 31);
                            //log_error(31); //falta ]
                            return 0;
                        }
                    } else {
                        test(toksig, vacio, 29);
                        // log_error(29); //falta [
                        return 0;
                    }
                } else {
                    test(toksig, vacio, 47);
                    //log_error(47); //falta token mientras
                    return 0;
                }
            } else {
                test(toksig, vacio, 22);
                //log_error(22); //falta }
                return 0;
            }
        } else {
            test(toksig, vacio, 21);
            //log_error(21); //falta {
            return 0;
        }
    }
    return sinError;
}

int instruccion_paraCada(struct nodoArchivo* miArchivo, struct clase *clase, struct metodo *metodo, int toksig[]) {
    int sinError = 1;
    int vacio[NOTOKENS];
    init_set(vacio);
    if (token == paracadaTok) {
        obtoken();
        if (token == corcheteI) {
            obtoken();
            int sigTipo[NOTOKENS];
            copia_set(sigTipo, toksig);
            sigTipo[ident] = 1;
            if (!tipo(miArchivo, clase, sigTipo)) {
                sinError = 0;
            }
            if (token == ident) {
                //verificar si exites
                obtoken();
                if (token == coma) {
                    obtoken();
                    if (token == ident) {
                        //verificar si existe 
                        obtoken();
                        if (token == corcheteF) {
                            obtoken();
                            if (token == llaveI) {
                                obtoken();
                                int sigIns[NOTOKENS];
                                copia_set(sigIns, toksig);
                                sigIns[llaveF] = 1;
                                if (!instruccion(miArchivo, clase, metodo, sigIns)) {
                                    sinError = 0;
                                }
                                if (token == llaveF) {
                                    obtoken();
                                } else {
                                    test(toksig, vacio, 22);
                                    //log_error(22); //falta }
                                    return 0;
                                }
                            } else {
                                test(toksig, vacio, 21);
                                //log_error(21); //falta {
                                return 0;
                            }
                        } else {
                            test(toksig, vacio, 31);
                            //log_error(31); //falta ]
                            return 0;
                        }
                    } else {
                        test(toksig, vacio, 18);
                        //log_error(18); //no es un identificador
                        return 0;
                    }
                } else {
                    test(toksig, vacio, 36);
                    //log_error(36); //falta ,
                    return 0;
                }
            } else {
                test(toksig, vacio, 18);
                //log_error(18); // no es un identificador
                return 0;
            }
        } else {

            test(toksig, vacio, 29);
            //log_error(29); // no esta el [
            return 0;
        }
    } else {
        test(toksig, vacio, 54);
        //log_error(54); //no esta token paracada
        return 0;
    }
    return sinError;
}
