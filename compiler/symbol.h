/******************************************************************************
 *  CVS version:
 *     $Id: symbol.h,v 1.1 2003/05/13 22:21:01 nickie Exp $
 ******************************************************************************
 *
 *  C header file : symbol.h
 *  Project       : PCL Compiler
 *  Version       : 1.0 alpha
 *  Written by    : Nikolaos S. Papaspyrou (nickie@softlab.ntua.gr)
 *  Date          : May 14, 2003
 *  Description   : Generic symbol table in C
 *
 *  Comments: (in Greek iso-8859-7)
 *  ---------
 *  Εθνικό Μετσόβιο Πολυτεχνείο.
 *  Σχολή Ηλεκτρολόγων Μηχανικών και Μηχανικών Υπολογιστών.
 *  Τομέας Τεχνολογίας Πληροφορικής και Υπολογιστών.
 *  Εργαστήριο Τεχνολογίας Λογισμικού
 */


#ifndef __SYMBOL_H__
#define __SYMBOL_H__


/* ---------------------------------------------------------------------
   -------------------------- Τύπος bool -------------------------------
   --------------------------------------------------------------------- */

#include <stdbool.h>
// #include "ast.h"
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Value.h>
#include <llvm/IR/Verifier.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Transforms/Scalar.h>
#if defined(LLVM_VERSION_MAJOR) && LLVM_VERSION_MAJOR >= 4
#include <llvm/Transforms/Scalar/GVN.h>
#endif

using namespace llvm;

/*
 *  Αν το παραπάνω include δεν υποστηρίζεται από την υλοποίηση
 *  της C που χρησιμοποιείτε, αντικαταστήστε το με το ακόλουθο:
 */

#if 0
typedef enum { false=0, true=1 } bool;
#endif


/* ---------------------------------------------------------------------
   ------------ Ορισμός σταθερών του πίνακα συμβόλων -------------------
   --------------------------------------------------------------------- */

#define START_POSITIVE_OFFSET 8     /* Αρχικό θετικό offset στο Ε.Δ.   */
#define START_NEGATIVE_OFFSET 0     /* Αρχικό αρνητικό offset στο Ε.Δ. */


/* ---------------------------------------------------------------------
   --------------- Ορισμός τύπων του πίνακα συμβόλων -------------------
   --------------------------------------------------------------------- */

/* Τύποι δεδομένων για την υλοποίηση των σταθερών */

typedef int           RepInteger;         /* Ακέραιες                  */
typedef unsigned char RepBoolean;         /* Λογικές τιμές             */
typedef char          RepChar;            /* Χαρακτήρες                */
typedef long double   RepReal;            /* Πραγματικές               */
typedef const char *  RepString;          /* Συμβολοσειρές             */


/* Τύποι δεδομένων και αποτελέσματος συναρτήσεων */

typedef struct Type_tag * my_Type;

typedef enum {                               /***** Το είδος του τύπου ****/
       TYPE_VOID,                        /* Κενός τύπος αποτελέσματος */
       TYPE_INTEGER,                     /* Ακέραιοι                  */
       TYPE_BOOLEAN,                     /* Λογικές τιμές             */
       TYPE_CHAR,                        /* Χαρακτήρες                */
       TYPE_REAL,                        /* Πραγματικοί               */
       TYPE_ARRAY,                       /* Πίνακες γνωστού μεγέθους  */
       TYPE_IARRAY,                      /* Πίνακες άγνωστου μεγέθους */
       TYPE_POINTER                      /* Δείκτες                   */
    } k;

typedef enum {                               /* Κατάσταση παραμέτρων  */
             PARDEF_COMPLETE,                    /* Πλήρης ορισμός     */
             PARDEF_DEFINE,                      /* Εν μέσω ορισμού    */
             PARDEF_CHECK                        /* Εν μέσω ελέγχου    */
         } pard;

struct Type_tag {
    k kind;
    my_Type           refType;              /* Τύπος αναφοράς            */
    RepInteger     size;                 /* Μέγεθος, αν είναι πίνακας */
    unsigned int   refCount;             /* Μετρητής αναφορών         */
};


/* Τύποι εγγραφών του πίνακα συμβόλων */

typedef enum {            
   ENTRY_VARIABLE,                       /* Μεταβλητές                 */
   ENTRY_CONSTANT,                       /* Σταθερές                   */
   ENTRY_FUNCTION,                       /* Συναρτήσεις                */
   ENTRY_PARAMETER,                      /* Παράμετροι συναρτήσεων     */
   ENTRY_TEMPORARY                       /* Προσωρινές μεταβλητές      */
} EntryType;


/* Τύποι περάσματος παραμετρων */

typedef enum {            
   PASS_BY_VALUE,                        /* Κατ' αξία                  */
   PASS_BY_REFERENCE                     /* Κατ' αναφορά               */
} PassMode;


/* Τύπος εγγραφής στον πίνακα συμβόλων */

typedef struct SymbolEntry_tag SymbolEntry;

typedef struct dimensions { 
        int arraysize;
            struct dimensions *next;
} *Dimensions;


struct SymbolEntry_tag {
   const char   * id;                 /* Ονομα αναγνωριστικού          */
   EntryType      entryType;          /* Τύπος της εγγραφής            */
   unsigned int   nestingLevel;       /* Βάθος φωλιάσματος             */
   unsigned int   hashValue;          /* Τιμή κατακερματισμού          */
   SymbolEntry  * nextHash;           /* Επόμενη εγγραφή στον Π.Κ.     */
   SymbolEntry  * nextInScope;        /* Επόμενη εγγραφή στην εμβέλεια */

   union {                            /* Ανάλογα με τον τύπο εγγραφής: */

      struct {                                /******* Μεταβλητή *******/
         my_Type               type;                  /* Τύπος                 */
         struct dimensions* dim;
         int                offset;                /* Offset στο Ε.Δ.       */
      } eVariable;

      struct {                                /******** Σταθερά ********/
         my_Type          type;                  /* Τύπος                 */
         union {                              /* Τιμή                  */
            RepInteger vInteger;              /*    ακέραια            */
            RepBoolean vBoolean;              /*    λογική             */
            RepChar    vChar;                 /*    χαρακτήρας         */
            RepReal    vReal;                 /*    πραγματική         */
            RepString  vString;               /*    συμβολοσειρά       */
         } value;
      } eConstant;

      struct {                                /******* Συνάρτηση *******/
         bool          isForward;             /* Δήλωση forward        */
         SymbolEntry * firstArgument;         /* Λίστα παραμέτρων      */
         SymbolEntry * lastArgument;          /* Τελευταία παράμετρος  */
         my_Type          resultType;            /* Τύπος αποτελέσματος   */
         pard pardef;
         int           firstQuad;             /* Αρχική τετράδα        */
         int 	       hasdeclared; 
	 Function*     func; 
      } eFunction;

      struct {                                /****** Παράμετρος *******/
         my_Type          type;                  /* Τύπος                 */
         struct dimensions* dim;
	     int           offset;                /* Offset στο Ε.Δ.       */
         PassMode      mode;                  /* Τρόπος περάσματος     */
         SymbolEntry * next;                  /* Επόμενη παράμετρος    */
         int           position;
      } eParameter;

      struct {                                /** Προσωρινή μεταβλητή **/
         my_Type          type;                  /* Τύπος                 */
         int           offset;                /* Offset στο Ε.Δ.       */
         int           number;
      } eTemporary;

   } u;                               /* Τέλος του union               */
};


/* Τύπος συνόλου εγγραφών που βρίσκονται στην ίδια εμβέλεια */

typedef struct Scope_tag Scope;

struct Scope_tag {
    unsigned int   nestingLevel;             /* Βάθος φωλιάσματος      */
    unsigned int   negOffset;                /* Τρέχον αρνητικό offset */
    Scope        * parent;                   /* Περιβάλλουσα εμβέλεια  */
    SymbolEntry  * entries;                  /* Σύμβολα της εμβέλειας  */
};


/* Τύπος αναζήτησης στον πίνακα συμβόλων */

typedef enum {
    LOOKUP_CURRENT_SCOPE,
    LOOKUP_ALL_SCOPES
} LookupType;


/* ---------------------------------------------------------------------
   ------------- Καθολικές μεταβλητές του πίνακα συμβόλων --------------
   --------------------------------------------------------------------- */

extern Scope        * currentScope;       /* Τρέχουσα εμβέλεια         */
extern unsigned int   quadNext;           /* Αριθμός επόμενης τετράδας */
extern unsigned int   tempNumber;         /* Αρίθμηση των temporaries  */

extern const my_Type typeVoid;
extern const my_Type typeInteger;
extern const my_Type typeBoolean;
extern const my_Type typeChar;
extern const my_Type typeReal;


/* ---------------------------------------------------------------------
   ------ Πρωτότυπα των συναρτήσεων χειρισμού του πίνακα συμβολών ------
   --------------------------------------------------------------------- */

void          initSymbolTable    (unsigned int size);
void          destroySymbolTable (void);

void          openScope          (void);
void          closeScope         (void);

SymbolEntry * newVariable        (const char * name, my_Type type, struct dimensions* dim);
SymbolEntry * newConstant        (const char * name, my_Type type, ...);
SymbolEntry * newFunction        (const char * name);
SymbolEntry * newParameter       (const char * name, my_Type type, Dimensions dim, int position,
                                  PassMode mode, SymbolEntry * f);
SymbolEntry * newTemporary       (my_Type type);

void          forwardFunction    (SymbolEntry * f);
void          endFunctionHeader  (SymbolEntry * f, my_Type type);
void          destroyEntry       (SymbolEntry * e);
SymbolEntry * lookupEntry        (const char * name, LookupType type,
                                  bool err);

my_Type          typeArray          (RepInteger size, my_Type refType);
my_Type          typeIArray         (my_Type refType);
my_Type          typePointer        (my_Type refType);
void          destroyType        (my_Type type);
unsigned int  sizeOfType         (my_Type type);
bool          equalType          (my_Type type1, my_Type type2);
void          printType          (my_Type type);
void          printMode          (PassMode mode);


#endif
