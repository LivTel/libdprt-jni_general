/* dprt_jni_general.c
** Common JNI routines used by dprt instrument libraries.
** $Header: /home/cjm/cvs/libdprt-jni_general/c/dprt_jni_general.c,v 1.1 2004-03-31 16:42:49 cjm Exp $
*/
/**
 * dprt_jni_general.c contains common JNI (Java Native Interface) routines over all dprt C libraries.
 * @author Chris Mottram, LJMU
 * @version $Revision: 1.1 $
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <jni.h>
#include "dprt_jni_general.h"

/* ------------------------------------------------------- */
/* hash definitions */
/* ------------------------------------------------------- */
/**
 * Property file name. This default value copied from the DpRtStatus.java source.
 */
#define PROPERTY_FILE_NAME "./dprt.properties"

/* ------------------------------------------------------- */
/* structure definitions */
/* ------------------------------------------------------- */
/**
 * Data type holding local data to dprt.c. This consists of the following:
 * <dl>
 * <dt>DpRt_Abort</dt><dd>Boolean value holding whether we should abort the current Data Reduction Pipeline. 
 * 	The reduction
 * 	routines should check this at regular intervals, and if it is TRUE
 * 	should abort processing and set the <a href="#DpRt_JNI_Error_Number">DpRt_JNI_Error_Number</a> and
 * 	<a href="#DpRt_JNI_Error_String">DpRt_JNI_Error_String</a> accordingly and return FALSE.</dd>
 * <dt>DpRt_Get_Property_Function_Pointer</dt><dd>Function pointer to actual routine that 
 * 	retrieves a keyword's string value from a Java property list Hashtable.</dd>
 * <dt>DpRt_Get_Property_Integer_Function_Pointer</dt><dd>Function pointer to actual routine that 
 * 	retrieves a keyword's integer value from a Java property list Hashtable.</dd>
 * <dt>DpRt_Get_Property_Double_Function_Pointer</dt><dd>Function pointer to actual routine that 
 * 	retrieves a keyword's double value from a Java property list Hashtable.</dd>
 * <dt>DpRt_Get_Property_Boolean_Function_Pointer</dt><dd>Function pointer to actual routine that retrieves 
 * 	a keyword's boolean value from a Java property list Hashtable.</dd>
 * <dt></dt><dd></dd>
 * </dl>
 * @see #DpRt_JNI_Error_Number
 * @see #DpRt_JNI_Error_String
 * @see #DpRt_JNI_Set_Abort
 * @see #DpRt_JNI_Get_Property
 * @see #DpRt_JNI_Get_Property_Integer
 * @see #DpRt_JNI_Get_Property_Double
 * @see #DpRt_JNI_Get_Property_Boolean
 */
struct DpRt_Struct
{
	volatile int DpRt_Abort;/* volatile as thread dependant */
	int (*DpRt_Get_Property_Function_Pointer)(char *keyword,char **value_string);
	int (*DpRt_Get_Property_Integer_Function_Pointer)(char *keyword,int *value);
	int (*DpRt_Get_Property_Double_Function_Pointer)(char *keyword,double *value);
	int (*DpRt_Get_Property_Boolean_Function_Pointer)(char *keyword,int *value);
};

/* ------------------------------------------------------- */
/* external variables */
/* ------------------------------------------------------- */
/**
 * Error Number - set this to a unique value for each location an error occurs.
 */
int DpRt_JNI_Error_Number = 0;
/**
 * Error String - set this to a descriptive string each place an error occurs.
 * Ensure the string is not longer than <a href="#DPRT_ERROR_STRING_LENGTH">DPRT_ERROR_STRING_LENGTH</a> long.
 * @see #DpRt_JNI_Error_Number
 * @see #DPRT_ERROR_STRING_LENGTH
 */
char DpRt_JNI_Error_String[DPRT_ERROR_STRING_LENGTH] = "";

/* ------------------------------------------------------- */
/* internal variables */
/* ------------------------------------------------------- */
/**
 * Revision Control System identifier.
 */
static char rcsid[] = "$Id: dprt_jni_general.c,v 1.1 2004-03-31 16:42:49 cjm Exp $";

/**
 * The single instance of struct DpRt_Struct, that holds local data to this source file.
 * Initialised to NULL/FALSE.
 * @see #DpRt_Struct
 */
static struct DpRt_Struct DpRt_Data = 
{
	FALSE,NULL,NULL,NULL,NULL
};

/**
 * Copy of the java virtual machine pointer, used for calling back up to the Java layer from C.
 */
static JavaVM *Java_VM = NULL;
/**
 * Cached global reference to the "DpRtLibrary" logger, used to log back to the Java layer from
 * C routines.
 */
static jobject Logger = NULL;
/**
 * Cached reference to the "ngat.util.logging.Logger" class's log(int level,String message) method.
 * Used to log C layer log messages, in conjunction with the logger's object reference logger.
 * @see #logger
 */
static jmethodID Log_Method_Id = NULL;
/**
 * Cached global reference to the "ngat.dprt.DpRtStatus" instance, so that C routines can access Java methods.
 */
static jobject DpRt_Status = NULL;
/**
 * Cached reference to the "ngat.dprt.DpRtStatus" class's getProperty(String keyword) method.
 * Used to retrieve configuration from the Java property file from the C layer.
 * @see #DpRt_Status
 */
static jmethodID DpRt_Status_Get_Property_Method_Id = NULL;
/**
 * Cached reference to the "ngat.dprt.DpRtStatus" class's getPropertyInteger(String keyword) method.
 * Used to retrieve configuration from the Java property file from the C layer.
 * @see #DpRt_Status
 */
static jmethodID DpRt_Status_Get_Property_Integer_Method_Id = NULL;
/**
 * Cached reference to the "ngat.dprt.DpRtStatus" class's getPropertyDouble(String keyword) method.
 * Used to retrieve configuration from the Java property file from the C layer.
 * @see #DpRt_Status
 */
static jmethodID DpRt_Status_Get_Property_Double_Method_Id = NULL;
/**
 * Cached reference to the "ngat.dprt.DpRtStatus" class's getPropertyBoolean(String keyword) method.
 * Used to retrieve configuration from the Java property file from the C layer.
 * @see #DpRt_Status
 */
static jmethodID DpRt_Status_Get_Property_Boolean_Method_Id = NULL;

/* ------------------------------------------------------- */
/* internal function declarations */
/* ------------------------------------------------------- */
static int DpRt_JNI_Get_Property_From_C_File(char *keyword,char **value_string);
static int DpRt_JNI_Get_Property_Integer_From_C_File(char *keyword,int *value);
static int DpRt_JNI_Get_Property_Double_From_C_File(char *keyword,double *value);
static int DpRt_JNI_Get_Property_Boolean_From_C_File(char *keyword,int *value);

/* ------------------------------------------------------- */
/* external functions */
/* ------------------------------------------------------- */
/* initialisation */
/**
 * This routine gets called when the native library is loaded. We use this routine
 * to get a copy of the JavaVM pointer of the JVM we are running in. This is used to
 * get the correct per-thread JNIEnv context pointer when C calls back into Java.
 * @see #Java_VM
 */
void DpRt_JNI_Set_Java_VM(JavaVM *vm)
{
	Java_VM = vm;
}

/**
 * This takes the supplied ngat.dprt.DpRtStatus object reference and stores it in the 
 * DpRt_Status variable as a global reference.
 * Some method ID's from this class are also retrieved and stored.
 * @param env The JNI environment pointer.
 * @param object The instance of ngat.dprt.DpRtLibraryInterface this method was called with.
 * @param status The DpRt's instance of ngat.dprt.DpRtStatus.
 * @see #DpRt_Status
 * @see #DpRt_Status_Get_Property_Method_Id
 * @see #DpRt_Status_Get_Property_Integer_Method_Id
 * @see #DpRt_Status_Get_Property_Double_Method_Id
 * @see #DpRt_Status_Get_Property_Boolean_Method_Id
 */
void DpRt_JNI_Set_Status(JNIEnv *env,jobject object,jobject status)
{
	jclass cls = NULL;

/* save DpRtStatus instance */
	DpRt_Status = (*env)->NewGlobalRef(env,status);
/* get the DpRtStatus class */
	cls = (*env)->FindClass(env,"ngat/dprt/DpRtStatus");
	/* if the class is null, one of the following exceptions occured:
	** ClassFormatError,ClassCircularityError,NoClassDefFoundError,OutOfMemoryError */
	if(cls == NULL)
		return;
/* get relevant method id's to call */
/* String getProperty(java/lang/String keyword) */
	DpRt_Status_Get_Property_Method_Id = (*env)->GetMethodID(env,cls,"getProperty",
								 "(Ljava/lang/String;)Ljava/lang/String;");
	if(DpRt_Status_Get_Property_Method_Id == NULL)
	{
		/* One of the following exceptions has been thrown:
		** NoSuchMethodError, ExceptionInInitializerError, OutOfMemoryError */
		return;
	}
/* int getPropertyInteger(java/lang/String keyword) */
	DpRt_Status_Get_Property_Integer_Method_Id = (*env)->GetMethodID(env,cls,"getPropertyInteger",
								 "(Ljava/lang/String;)I");
	if(DpRt_Status_Get_Property_Integer_Method_Id == NULL)
	{
		/* One of the following exceptions has been thrown:
		** NoSuchMethodError, ExceptionInInitializerError, OutOfMemoryError */
		return;
	}
/* double getPropertyDouble(java/lang/String keyword) */
	DpRt_Status_Get_Property_Double_Method_Id = (*env)->GetMethodID(env,cls,"getPropertyDouble",
								 "(Ljava/lang/String;)D");
	if(DpRt_Status_Get_Property_Double_Method_Id == NULL)
	{
		/* One of the following exceptions has been thrown:
		** NoSuchMethodError, ExceptionInInitializerError, OutOfMemoryError */
		return;
	}
/* boolean getPropertyBoolean(java/lang/String keyword) */
	DpRt_Status_Get_Property_Boolean_Method_Id = (*env)->GetMethodID(env,cls,"getPropertyBoolean",
								 "(Ljava/lang/String;)Z");
	if(DpRt_Status_Get_Property_Boolean_Method_Id == NULL)
	{
		/* One of the following exceptions has been thrown:
		** NoSuchMethodError, ExceptionInInitializerError, OutOfMemoryError */
		return;
	}
}

/**
 * This takes the supplied logger object reference and stores it in the logger variable as a global reference.
 * The log method ID is also retrieved and stored.
 * @param l The DpRtLibrary's specific (instrument specific) logger.
 * @see #Logger
 * @see #Log_Method_Id
 */
void DpRt_JNI_Initialise_Logger_Reference(JNIEnv *env,jobject obj,jobject l)
{
	jclass cls = NULL;

/* save logger instance */
	Logger = (*env)->NewGlobalRef(env,l);
/* get the ngat.util.logging.Logger class */
	cls = (*env)->FindClass(env,"ngat/util/logging/Logger");
	/* if the class is null, one of the following exceptions occured:
	** ClassFormatError,ClassCircularityError,NoClassDefFoundError,OutOfMemoryError */
	if(cls == NULL)
		return;
/* get relevant method id to call */
/* log(int level,java/lang/String message) */
	Log_Method_Id = (*env)->GetMethodID(env,cls,"log","(ILjava/lang/String;)V");
	if(Log_Method_Id == NULL)
	{
		/* One of the following exceptions has been thrown:
		** NoSuchMethodError, ExceptionInInitializerError, OutOfMemoryError */
		return;
	}
}

/**
 * This native method is called from DpRtLibrary's instrumetn specific finaliser method. 
 * It removes the global reference to logger.
 * @see #Logger
 */
void DpRt_JNI_Finalise_Logger_Reference(JNIEnv *env)
{
	(*env)->DeleteGlobalRef(env,Logger);
}

/**
 * Routine called as DpRt is finalised, to clear up DpRt_Status global reference.
 * @param env The JNI environment pointer.
 * @see #DpRt_Status
 */
void DpRt_JNI_Finalise_Status_Reference(JNIEnv *env)
{
	if(DpRt_Status != NULL)
		(*env)->DeleteGlobalRef(env,DpRt_Status);
}

/**
 * This finction should be called when the library/DpRt is first initialised/loaded.
 * It allows the C layer to perform initial initialisation.
 * The function pointers to use a C routine to load the property from the config file are initialised.
 * Note these function pointers will be over-written by the functions in DpRtLibrary.c if this
 * initialise routine was called from the Java (JNI) layer.
 * @see #DpRt_JNI_Set_Property_Function_Pointer
 * @see #DpRt_JNI_Set_Property_Integer_Function_Pointer
 * @see #DpRt_JNI_Set_Property_Double_Function_Pointer
 * @see #DpRt_JNI_Set_Property_Boolean_Function_Pointer
 * @see #DpRt_JNI_Get_Property_From_C_File
 * @see #DpRt_JNI_Get_Property_Integer_From_C_File
 * @see #DpRt_JNI_Get_Property_Double_From_C_File
 * @see #DpRt_JNI_Get_Property_Boolean_From_C_File
 */
int DpRt_JNI_Initialise(void)
{
	DpRt_JNI_Error_Number = 0;
	DpRt_JNI_Error_String[0] = '\0';
	if(DpRt_Data.DpRt_Get_Property_Function_Pointer == NULL)
		DpRt_JNI_Set_Property_Function_Pointer(DpRt_JNI_Get_Property_From_C_File);
	if(DpRt_Data.DpRt_Get_Property_Integer_Function_Pointer == NULL)
		DpRt_JNI_Set_Property_Integer_Function_Pointer(DpRt_JNI_Get_Property_Integer_From_C_File);
	if(DpRt_Data.DpRt_Get_Property_Double_Function_Pointer == NULL)
		DpRt_JNI_Set_Property_Double_Function_Pointer(DpRt_JNI_Get_Property_Double_From_C_File);
	if(DpRt_Data.DpRt_Get_Property_Boolean_Function_Pointer == NULL)
		DpRt_JNI_Set_Property_Boolean_Function_Pointer(DpRt_JNI_Get_Property_Boolean_From_C_File);
	return TRUE;
}

/* abort */
/**
 * A routine to set the value of the DpRt_Abort variable. This is used to
 * keep track of whether an abort event has occured.
 * @param value The value to set the abort value to, this should be TRUE of we want the data reduction
 * aborted, or FALSE if we don't or we are resetting the flag.
 * @see #DpRt_Data
 */
void DpRt_JNI_Set_Abort(int value)
{
	DpRt_Data.DpRt_Abort = value;
}

/**
 * A routine to get the current value of the abort variable, to determine whether we should abort
 * processing the FITS file or not.
 * @return The current value of the DpRt_Data.DpRt_Abort variable, usually TRUE if we want to abort a reduction process
 * and FALSE if we don't.
 * @see #DpRt_Data
 */
int DpRt_JNI_Get_Abort(void)
{
	return DpRt_Data.DpRt_Abort;
}

/* property file processing */
/* top level client API */
/**
 * This routine allows us to query the properties loaded into DpRt to get
 * the value associated with a keyword in the property Hashtable.
 * As libdprt can be called in two ways, from Java using JNI (the usual method) and
 * from a C test program (for testing), the mechanism for retrieving the value is different
 * in each case. A function pointer system is used.
 * @param keyword The keyword in the property file to look up.
 * @param value_string The address of a pointer to allocate and store the resulting value string in.
 * 	This pointer is dynamically allocated and must be freed using <b>free()</b>. 
 * @return The routine returns TRUE if it succeeds, FALSE if it fails.
 * @see #DpRt_Data
 */
int DpRt_JNI_Get_Property(char *keyword,char **value_string)
{
	DpRt_JNI_Error_Number = 0;
	DpRt_JNI_Error_String[0] = '\0';
	if(keyword == NULL)
	{
		DpRt_JNI_Error_Number = 5;
		sprintf(DpRt_JNI_Error_String,"DpRt_Get_Property failed: Keyword was NULL.\n");
		return FALSE;
	}
	if(value_string == NULL)
	{
		DpRt_JNI_Error_Number = 6;
		sprintf(DpRt_JNI_Error_String,"DpRt_Get_Property failed: Value String Pointer was NULL.\n");
		return FALSE;
	}
	if(DpRt_Data.DpRt_Get_Property_Function_Pointer == NULL)
	{
		DpRt_JNI_Error_Number = 7;
		sprintf(DpRt_JNI_Error_String,"DpRt_Get_Property failed: Function Pointer was NULL.\n");
		return FALSE;
	}
	return DpRt_Data.DpRt_Get_Property_Function_Pointer(keyword,value_string);
}

/**
 * This routine allows us to query the properties loaded into DpRt to get
 * the integer value associated with a keyword in the property Hashtable.
 * As libdprt can be called in two ways, from Java using JNI (the usual method) and
 * from a C test program (for testing), the mechanism for retrieving the value is different
 * in each case. A function pointer system is used.
 * @param keyword The keyword in the property file to look up.
 * @param value The address of an integer to store the resulting value.
 * @return The routine returns TRUE if it succeeds, FALSE if it fails.
 * @see #DpRt_Data
 */
int DpRt_JNI_Get_Property_Integer(char *keyword,int *value)
{
	DpRt_JNI_Error_Number = 0;
	DpRt_JNI_Error_String[0] = '\0';
	if(keyword == NULL)
	{
		DpRt_JNI_Error_Number = 8;
		sprintf(DpRt_JNI_Error_String,"DpRt_Get_Property_Integer failed: Keyword was NULL.\n");
		return FALSE;
	}
	if(value == NULL)
	{
		DpRt_JNI_Error_Number = 9;
		sprintf(DpRt_JNI_Error_String,"DpRt_Get_Property_Integer failed: Value Pointer was NULL.\n");
		return FALSE;
	}
	if(DpRt_Data.DpRt_Get_Property_Integer_Function_Pointer == NULL)
	{
		DpRt_JNI_Error_Number = 10;
		sprintf(DpRt_JNI_Error_String,"DpRt_Get_Property_Integer failed: Function Pointer was NULL.\n");
		return FALSE;
	}
	return DpRt_Data.DpRt_Get_Property_Integer_Function_Pointer(keyword,value);
}

/**
 * This routine allows us to query the properties loaded into DpRt to get
 * the double value associated with a keyword in the property Hashtable.
 * As libdprt can be called in two ways, from Java using JNI (the usual method) and
 * from a C test program (for testing), the mechanism for retrieving the value is different
 * in each case. A function pointer system is used.
 * @param keyword The keyword in the property file to look up.
 * @param value The address of an double to store the resulting value.
 * @return The routine returns TRUE if it succeeds, FALSE if it fails.
 * @see #DpRt_Data
 */
int DpRt_JNI_Get_Property_Double(char *keyword,double *value)
{
	DpRt_JNI_Error_Number = 0;
	DpRt_JNI_Error_String[0] = '\0';
	if(keyword == NULL)
	{
		DpRt_JNI_Error_Number = 11;
		sprintf(DpRt_JNI_Error_String,"DpRt_Get_Property_Double failed: Keyword was NULL.\n");
		return FALSE;
	}
	if(value == NULL)
	{
		DpRt_JNI_Error_Number = 12;
		sprintf(DpRt_JNI_Error_String,"DpRt_Get_Property_Double failed: Value Pointer was NULL.\n");
		return FALSE;
	}
	if(DpRt_Data.DpRt_Get_Property_Double_Function_Pointer == NULL)
	{
		DpRt_JNI_Error_Number = 13;
		sprintf(DpRt_JNI_Error_String,"DpRt_Get_Property_Double failed: Function Pointer was NULL.\n");
		return FALSE;
	}
	return DpRt_Data.DpRt_Get_Property_Double_Function_Pointer(keyword,value);
}

/**
 * This routine allows us to query the properties loaded into DpRt to get
 * the boolean value associated with a keyword in the property Hashtable.
 * As libdprt can be called in two ways, from Java using JNI (the usual method) and
 * from a C test program (for testing), the mechanism for retrieving the value is different
 * in each case. A function pointer system is used.
 * @param keyword The keyword in the property file to look up.
 * @param value The address of an integer to store the resulting value, 1 is TRUE, 0 is FALSE.
 * @return The routine returns TRUE if it succeeds, FALSE if it fails.
 * @see #DpRt_Data
 */
int DpRt_JNI_Get_Property_Boolean(char *keyword,int *value)
{
	DpRt_JNI_Error_Number = 0;
	DpRt_JNI_Error_String[0] = '\0';
	if(keyword == NULL)
	{
		DpRt_JNI_Error_Number = 14;
		sprintf(DpRt_JNI_Error_String,"DpRt_Get_Property_Boolean failed: Keyword was NULL.\n");
		return FALSE;
	}
	if(value == NULL)
	{
		DpRt_JNI_Error_Number = 15;
		sprintf(DpRt_JNI_Error_String,"DpRt_Get_Property_Boolean failed: Value Pointer was NULL.\n");
		return FALSE;
	}
	if(DpRt_Data.DpRt_Get_Property_Boolean_Function_Pointer == NULL)
	{
		DpRt_JNI_Error_Number = 16;
		sprintf(DpRt_JNI_Error_String,"DpRt_Get_Property_Boolean failed: Function Pointer was NULL.\n");
		return FALSE;
	}
	return DpRt_Data.DpRt_Get_Property_Boolean_Function_Pointer(keyword,value);
}

/* routines to access proerties via DpRtStatus object.
** external, as can be passed as parameters to DpRt_JNI_Set_Property_*_Function_Pointer */
/**
 * Routine to get the value of the keyword from the properties held in the instance of DpRtStatus.
 * @param keyword The keyword in the property file to look up.
 * @param value_string The address of a pointer to allocate and store the resulting value string in.
 * 	This pointer is dynamically allocated and must be freed using <b>free()</b>. 
 * @see #DpRt_Status
 * @see #DpRt_Status_Get_Property_Method_Id
 */
int DpRt_JNI_DpRtStatus_Get_Property(char *keyword,char **value_string)
{
	JNIEnv *env = NULL;
	jstring java_keyword_string = NULL;
	jobject java_value_object = NULL;
	jstring java_value_string = NULL;
	const char *c_value_string = NULL;

	if(DpRt_Status == NULL)
	{
		DpRt_JNI_Error_Number = 1;
		sprintf(DpRt_JNI_Error_String,"DpRt_JNI_DpRtStatus_Get_Property:DpRt_Status was NULL (%s).\n",
			keyword);
		return FALSE;
	}
	if(DpRt_Status_Get_Property_Method_Id == NULL)
	{
		DpRt_JNI_Error_Number = 2;
		sprintf(DpRt_JNI_Error_String,"DpRt_JNI_DpRtStatus_Get_Property:Method ID was NULL (%s).\n",
			keyword);
		return FALSE;
	}
	if(Java_VM == NULL)
	{
		DpRt_JNI_Error_Number = 3;
		sprintf(DpRt_JNI_Error_String,"DpRt_JNI_DpRtStatus_Get_Property:Java_VM was NULL (%s).\n",
			keyword);
		return FALSE;
	}
/* get java env for this thread */
	(*Java_VM)->AttachCurrentThread(Java_VM,(void**)&env,NULL);
	if(env == NULL)
	{
		DpRt_JNI_Error_Number = 4;
		sprintf(DpRt_JNI_Error_String,"DpRt_JNI_DpRtStatus_Get_Property:env was NULL (%s).\n",keyword);
		return FALSE;
	}
	if(keyword == NULL)
	{
		DpRt_JNI_Error_Number = 5;
		sprintf(DpRt_JNI_Error_String,"DpRt_JNI_DpRtStatus_Get_Property:string (%s) was NULL.\n",keyword);
		return FALSE;
	}
	if(value_string == NULL)
	{
		DpRt_JNI_Error_Number = 6;
		sprintf(DpRt_JNI_Error_String,"DpRt_JNI_DpRtStatus_Get_Property:value_string Pointer was NULL.\n");
		return FALSE;
	}
/* convert C to Java String */
	java_keyword_string = (*env)->NewStringUTF(env,keyword);
/* call getProperty method on DpRt_Status instance */
	java_value_object = (*env)->CallObjectMethod(env,DpRt_Status,DpRt_Status_Get_Property_Method_Id,
			java_keyword_string);
/* Convert Java JString to C character array */
	java_value_string = (jstring)java_value_object;
	if(java_value_string != NULL)
		c_value_string = (*env)->GetStringUTFChars(env,java_value_string,0);
/* copy c_value_string to (*value_string) */
	if(c_value_string != NULL)
	{
		(*value_string) = (char *)malloc((strlen(c_value_string)+1)*sizeof(char));
		if((*value_string) == NULL)
		{
			DpRt_JNI_Error_Number = 7;
			sprintf(DpRt_JNI_Error_String,"DpRt_JNI_DpRtStatus_Get_Property:"
				"Memory allocation error(%s,%d).\n",
				c_value_string,strlen(c_value_string));
			return FALSE;
		}
		strcpy((*value_string),c_value_string);
	}
	else
		(*value_string) = NULL;
/* free c_value_string */
	if(java_value_string != NULL)
		(*env)->ReleaseStringUTFChars(env,java_value_string,c_value_string);
	return TRUE;
}

/**
 * Routine to get the integer value of the keyword from the properties held in the instance of DpRtStatus.
 * @param keyword The keyword in the property file to look up.
 * @param value The address of an integer to store the resulting integer value in.
 * @see #DpRt_Status
 * @see #DpRt_Status_Get_Property_Integer_Method_Id
 */
int DpRt_JNI_DpRtStatus_Get_Property_Integer(char *keyword,int *value)
{
	JNIEnv *env = NULL;
	jstring java_keyword_string = NULL;

	if(DpRt_Status == NULL)
	{
		DpRt_JNI_Error_Number = 8;
		sprintf(DpRt_JNI_Error_String,"DpRt_JNI_DpRtStatus_Get_Property_Integer:"
			"DpRt_Status was NULL (%s).\n",
			keyword);
		return FALSE;
	}
	if(DpRt_Status_Get_Property_Integer_Method_Id == NULL)
	{
		DpRt_JNI_Error_Number = 9;
		sprintf(DpRt_JNI_Error_String,"DpRt_JNI_DpRtStatus_Get_Property_Integer:Method ID was NULL(%s).\n",
			keyword);
		return FALSE;
	}
	if(Java_VM == NULL)
	{
		DpRt_JNI_Error_Number = 10;
		sprintf(DpRt_JNI_Error_String,"DpRt_JNI_DpRtStatus_Get_Property_Integer:Java_VM was NULL(%s).\n",
			keyword);
		return FALSE;
	}
/* get java env for this thread */
	(*Java_VM)->AttachCurrentThread(Java_VM,(void**)&env,NULL);
	if(env == NULL)
	{
		DpRt_JNI_Error_Number = 11;
		sprintf(DpRt_JNI_Error_String,"DpRt_JNI_DpRtStatus_Get_Property_Integer:env was NULL (%s).\n",
			keyword);
		return FALSE;
	}
	if(keyword == NULL)
	{
		DpRt_JNI_Error_Number = 12;
		sprintf(DpRt_JNI_Error_String,"DpRt_JNI_DpRtStatus_Get_Property_Integer:string (%s) was NULL.\n",
			keyword);
		return FALSE;
	}
	if(value == NULL)
	{
		DpRt_JNI_Error_Number = 13;
		sprintf(DpRt_JNI_Error_String,"DpRt_JNI_DpRtStatus_Get_Property_Integer:value Pointer was NULL.\n");
		return FALSE;
	}
/* convert C to Java String */
	java_keyword_string = (*env)->NewStringUTF(env,keyword);
/* call getProperty method on DpRt_Status instance */
	(*value) = (int)((*env)->CallIntMethod(env,DpRt_Status,DpRt_Status_Get_Property_Integer_Method_Id,
			java_keyword_string));
	return TRUE;
}

/**
 * Routine to get the double value of the keyword from the properties held in the instance of DpRtStatus.
 * @param keyword The keyword in the property file to look up.
 * @param value The address of an double to store the resulting value in.
 * @see #DpRt_Status
 * @see #DpRt_Status_Get_Property_Double_Method_Id
 */
int DpRt_JNI_DpRtStatus_Get_Property_Double(char *keyword,double *value)
{
	JNIEnv *env = NULL;
	jstring java_keyword_string = NULL;

	if(DpRt_Status == NULL)
	{
		DpRt_JNI_Error_Number = 14;
		sprintf(DpRt_JNI_Error_String,"DpRt_JNI_DpRtStatus_Get_Property_Double:DpRt_Status was NULL(%s).\n",
			keyword);
		return FALSE;
	}
	if(DpRt_Status_Get_Property_Double_Method_Id == NULL)
	{
		DpRt_JNI_Error_Number = 15;
		sprintf(DpRt_JNI_Error_String,"DpRt_JNI_DpRtStatus_Get_Property_Double:Method ID was NULL (%s).\n",
			keyword);
		return FALSE;
	}
	if(Java_VM == NULL)
	{
		DpRt_JNI_Error_Number = 16;
		sprintf(DpRt_JNI_Error_String,"DpRt_JNI_DpRtStatus_Get_Property_Double:Java_VM was NULL(%s).\n",
			keyword);
		return FALSE;
	}
/* get java env for this thread */
	(*Java_VM)->AttachCurrentThread(Java_VM,(void**)&env,NULL);
	if(env == NULL)
	{
		DpRt_JNI_Error_Number = 17;
		sprintf(DpRt_JNI_Error_String,"DpRt_JNI_DpRtStatus_Get_Property_Double:env was NULL (%s).\n",
			keyword);
		return FALSE;
	}
	if(keyword == NULL)
	{
		DpRt_JNI_Error_Number = 18;
		sprintf(DpRt_JNI_Error_String,"DpRt_JNI_DpRtStatus_Get_Property_Double:string (%s) was NULL.\n",
			keyword);
		return FALSE;
	}
	if(value == NULL)
	{
		DpRt_JNI_Error_Number = 19;
		sprintf(DpRt_JNI_Error_String,"DpRt_JNI_DpRtStatus_Get_Property_Double:value Pointer was NULL.\n");
		return FALSE;
	}
/* convert C to Java String */
	java_keyword_string = (*env)->NewStringUTF(env,keyword);
/* call getProperty method on DpRt_Status instance */
	(*value) = (double)((*env)->CallDoubleMethod(env,DpRt_Status,DpRt_Status_Get_Property_Double_Method_Id,
			java_keyword_string));
	return TRUE;
}

/**
 * Routine to get the boolean value of the keyword from the properties held in the instance of DpRtStatus.
 * @param keyword The keyword in the property file to look up.
 * @param value The address of an boolean to store the resulting value in.
 * @see #DpRt_Status
 * @see #DpRt_Status_Get_Property_Boolean_Method_Id
 */
int DpRt_JNI_DpRtStatus_Get_Property_Boolean(char *keyword,int *value)
{
	JNIEnv *env = NULL;
	jstring java_keyword_string = NULL;
	jboolean boolean_value;

	if(DpRt_Status == NULL)
	{
		DpRt_JNI_Error_Number = 20;
		sprintf(DpRt_JNI_Error_String,"DpRt_JNI_DpRtStatus_Get_Property_Boolean:"
			"DpRt_Status was NULL(%s).\n",keyword);
		return FALSE;
	}
	if(DpRt_Status_Get_Property_Boolean_Method_Id == NULL)
	{
		DpRt_JNI_Error_Number = 21;
		sprintf(DpRt_JNI_Error_String,"DpRt_JNI_DpRtStatus_Get_Property_Boolean:Method ID was NULL(%s).\n",
			keyword);
		return FALSE;
	}
	if(Java_VM == NULL)
	{
		DpRt_JNI_Error_Number = 22;
		sprintf(DpRt_JNI_Error_String,"DpRt_JNI_DpRtStatus_Get_Property_Boolean:Java_VM was NULL(%s).\n",
			keyword);
		return FALSE;
	}
/* get java env for this thread */
	(*Java_VM)->AttachCurrentThread(Java_VM,(void**)&env,NULL);
	if(env == NULL)
	{
		DpRt_JNI_Error_Number = 23;
		sprintf(DpRt_JNI_Error_String,"DpRt_JNI_DpRtStatus_Get_Property_Boolean:env was NULL (%s).\n",
			keyword);
		return FALSE;
	}
	if(keyword == NULL)
	{
		DpRt_JNI_Error_Number = 24;
		sprintf(DpRt_JNI_Error_String,"DpRt_JNI_DpRtStatus_Get_Property_Boolean:string (%s) was NULL.\n",
			keyword);
		return FALSE;
	}
	if(value == NULL)
	{
		DpRt_JNI_Error_Number = 25;
		sprintf(DpRt_JNI_Error_String,"DpRt_JNI_DpRtStatus_Get_Property_Boolean:value Pointer was NULL.\n");
		return FALSE;
	}
/* convert C to Java String */
	java_keyword_string = (*env)->NewStringUTF(env,keyword);
/* call getProperty method on DpRt_Status instance */
	boolean_value = (double)((*env)->CallBooleanMethod(env,DpRt_Status,DpRt_Status_Get_Property_Boolean_Method_Id,
			java_keyword_string));
	if(boolean_value)
		(*value) = TRUE;
	else
		(*value) = FALSE;
	return TRUE;
}

/* set property function pointers */
/**
 * Routine to set the function pointer that is called from <b>DpRt_Get_Property</b> .
 * @see #DpRt_Get_Property
 * @see #DpRt_Data
 */
void DpRt_JNI_Set_Property_Function_Pointer(int (*get_property_fp)(char *keyword,char **value_string))
{
	DpRt_Data.DpRt_Get_Property_Function_Pointer = get_property_fp;
}

/**
 * Routine to set the function pointer that is called from <b>DpRt_Get_Property_Integer</b> .
 * @see #DpRt_Get_Property_Integer
 * @see #DpRt_Data
 */
void DpRt_JNI_Set_Property_Integer_Function_Pointer(int (*get_property_integer_fp)(char *keyword,int *value))
{
	DpRt_Data.DpRt_Get_Property_Integer_Function_Pointer = get_property_integer_fp;
}

/**
 * Routine to set the function pointer that is called from <b>DpRt_Get_Property_Double</b> .
 * @see #DpRt_Get_Property_Double
 * @see #DpRt_Data
 */
void DpRt_JNI_Set_Property_Double_Function_Pointer(int (*get_property_double_fp)(char *keyword,double *value))
{
	DpRt_Data.DpRt_Get_Property_Double_Function_Pointer = get_property_double_fp;
}

/**
 * Routine to set the function pointer that is called from <b>DpRt_Get_Property_Boolean</b> .
 * @see #DpRt_Get_Property_Boolean
 * @see #DpRt_Data
 */
void DpRt_JNI_Set_Property_Boolean_Function_Pointer(int (*get_property_boolean_fp)(char *keyword,int *value))
{
	DpRt_Data.DpRt_Get_Property_Boolean_Function_Pointer = get_property_boolean_fp;
}

/* command done */
/**
 * Routine to set the COMMAND_DONE return parameters for a JNI command.
 * @param env The usual JNI parameter.
 * @param cls The JNI class identifier to get the methods for.
 * @param done The object to call the methods for.
 * @param successful The value to set the COMMAND_DONE.successful to.
 * @param error_number The value to set the COMMAND_DONE.errorNumber to.
 * @param error_string The value to set the COMMAND_DONE.errorString to.
 * @return TRUE if all the methods were called successfully, FALSE if a method call failed.
 */
int DpRt_JNI_Set_Command_Done(JNIEnv *env,jclass cls,jobject done,
					int successful,int error_number,char *error_string)
{
	jmethodID mid;

	/* successful */
	/* get the method id in this class */
	mid = (*env)->GetMethodID(env,cls,"setSuccessful","(Z)V");
	/* did we find the method id? */
	if (mid == 0)
		return FALSE;
	/* call the method */
	(*env)->CallVoidMethod(env,done,mid,successful);

	/* error number */
	/* get the method id in this class */
	mid = (*env)->GetMethodID(env,cls,"setErrorNum","(I)V");
	/* did we find the method id? */
	if (mid == 0)
		return FALSE;
	/* call the method */
	(*env)->CallVoidMethod(env,done,mid,error_number);

	/* error string */
	/* get the method id in this class */
	mid = (*env)->GetMethodID(env,cls,"setErrorString","(Ljava/lang/String;)V");
	/* did we find the method id? */
	if (mid == 0)
		return FALSE;
	/* call the method */
	(*env)->CallVoidMethod(env,done,mid,(*env)->NewStringUTF(env,error_string));

	return TRUE;
}

/**
 * Internal routine to set the REDUCE_DONE return parameters for a JNI command.
 * @param env The usual JNI parameter.
 * @param cls The JNI class identifier to get the methods for.
 * @param done The object to call the methods for.
 * @param output_filename The value to set the REDUCE_DONE.filename to.
 * @return TRUE if all the methods were called successfully, FALSE if a method call failed.
 */
int DpRt_JNI_Set_Reduce_Done(JNIEnv *env,jclass cls,jobject done,char *output_filename)
{
	jmethodID mid;

	/* output_filename */
	/* get the method id in this class */
	mid = (*env)->GetMethodID(env,cls,"setFilename","(Ljava/lang/String;)V");
	/* did we find the method id? */
	if (mid == 0)
		return FALSE;
	/* call the method */
	if(output_filename != NULL)
		(*env)->CallVoidMethod(env,done,mid,(*env)->NewStringUTF(env,output_filename));
	else
		(*env)->CallVoidMethod(env,done,mid,NULL);
	return TRUE;
}

/**
 * Routine to set the CALIBRATE_REDUCE_DONE return parameters for a JNI command.
 * @param env The usual JNI parameter.
 * @param cls The JNI class identifier to get the methods for.
 * @param done The object to call the methods for. Should be an instance of CALIBRATE_REDUCE_DONE.
 * @param mean_counts The mean counts parameter.
 * @param peak_counts The peak counts parameter.
 * @return TRUE if all the methods were called successfully, FALSE if a method call failed.
 */
int DpRt_JNI_Set_Calibrate_Reduce_Done(JNIEnv *env,jclass cls,jobject done,
					double mean_counts,double peak_counts)
{
	jmethodID mid;

	/* meanCounts */
	/* get the method id in this class */
	mid = (*env)->GetMethodID(env,cls,"setMeanCounts","(F)V");
	/* did we find the method id? */
	if (mid == 0)
		return FALSE;
	/* call the method	*/
	(*env)->CallVoidMethod(env,done,mid,(float)mean_counts);

	/* peakCounts */
	/* get the method id in this class */
	mid = (*env)->GetMethodID(env,cls,"setPeakCounts","(F)V");
	/* did we find the method id? */
	if (mid == 0)
		return FALSE;
	/* call the method */
	(*env)->CallVoidMethod(env,done,mid,(float)peak_counts);
	return TRUE;
}

/**
 * Routine to set the EXPOSE_REDUCE_DONE return parameters for a JNI command.
 * @param env The usual JNI parameter.
 * @param cls The JNI class identifier to get the methods for.
 * @param done The object to call the methods for. Should be an instance of EXPOSE_REDUCE_DONE.
 * @param seeing The seeing parameter.
 * @param counts The counts of the brightest object parameter.
 * @param x_pix The X position in pixels of the brightest object.
 * @param y_pix The Y position in pixels of the brightest object.
 * @param photometricity A measure of the photometricity of the field (if a standard).
 * @param sky_brightness A measure of the sky brightness.
 * @param saturated An integer (boolean), set to TRUE if the field contains saturated stars.
 * @return TRUE if all the methods were called successfully, FALSE if a method call failed.
 */
int DpRt_JNI_Set_Expose_Reduce_Done(JNIEnv *env,jclass cls,jobject done,double seeing,double counts,
				    double x_pix,double y_pix,double photometricity,
				    double sky_brightness,int saturated)
{
	jmethodID mid;

	/* seeing */
	/* get the method id in this class */
	mid = (*env)->GetMethodID(env,cls,"setSeeing","(F)V");
	/* did we find the method id? */
	if (mid == 0)
		return FALSE;
	/* call the method */
	(*env)->CallVoidMethod(env,done,mid,(float)seeing);

	/* counts */
	/* get the method id in this class */
	mid = (*env)->GetMethodID(env,cls,"setCounts","(F)V");
	/* did we find the method id? */
	if (mid == 0)
		return FALSE;
	/* call the method */
	(*env)->CallVoidMethod(env,done,mid,(float)counts);

	/* x_pix */
	/* get the method id in this class */
	mid = (*env)->GetMethodID(env,cls,"setXpix","(F)V");
	/* did we find the method id? */
	if (mid == 0)
		return FALSE;
	/* call the method */
	(*env)->CallVoidMethod(env,done,mid,(float)x_pix);

	/* y_pix */
	/* get the method id in this class */
	mid = (*env)->GetMethodID(env,cls,"setYpix","(F)V");
	/* did we find the method id? */
	if (mid == 0)
		return FALSE;
	/* call the method */
	(*env)->CallVoidMethod(env,done,mid,(float)y_pix);

	/* photometricity */
	/* get the method id in this class */
	mid = (*env)->GetMethodID(env,cls,"setPhotometricity","(F)V");
	/* did we find the method id? */
	if (mid == 0)
		return FALSE;
	/* call the method */
	(*env)->CallVoidMethod(env,done,mid,(float)photometricity);

	/* sky_brightness */
	/* get the method id in this class */
	mid = (*env)->GetMethodID(env,cls,"setSkyBrightness","(F)V");
	/* did we find the method id? */
	if (mid == 0)
		return FALSE;
	/* call the method */
	(*env)->CallVoidMethod(env,done,mid,(float)sky_brightness);

	/* saturated */
	/* get the method id in this class */
	mid = (*env)->GetMethodID(env,cls,"setSaturation","(Z)V");
	/* did we find the method id? */
	if (mid == 0)
		return FALSE;
	/* call the method */
	(*env)->CallVoidMethod(env,done,mid,(jboolean)saturated);
	return TRUE;
}


/* exception handling */
/**
 * This routine throws an exception. The error generated is from the error codes in dprt, it assumes
 * another routine has generated an error and this routine packs this error into an exception to return
 * to the Java code, using DpRt_JNI_Throw_Exception_String. The total length of the error string should
 * not be longer than DPRT_ERROR_STRING_LENGTH. A new line is added to the start of the error string,
 * so that the error string returned from libdprt is formatted properly.
 * @param env The JNI environment pointer.
 * @param function_name The name of the function in which this exception is being generated for.
 * @see #DpRt_JNI_Get_Error_Number
 * @see #DpRt_JNI_Get_Error_String
 * @see #DpRt_JNI_Throw_Exception_String
 * @see #DPRT_ERROR_STRING_LENGTH
 */
void DpRt_JNI_Throw_Exception(JNIEnv *env,char *function_name)
{
	char error_string[DPRT_ERROR_STRING_LENGTH];
	int error_number;

	error_number = DpRt_JNI_Get_Error_Number();
	strcpy(error_string,"\n");
	DpRt_JNI_Get_Error_String(error_string+strlen(error_string));
	DpRt_JNI_Throw_Exception_String(env,function_name,error_number,error_string);
}

/**
 * This routine throws an exception of class ngat.dprt.ccs.DpRtLibraryNativeException. This is used to report
 * all libdprt error messages back to the Java layer.
 * @param env The JNI environment pointer.
 * @param function_name The name of the function in which this exception is being generated for.
 * @param error_number The error number to pass to the constructor of the exception.
 * @param error_string The string to pass to the constructor of the exception.
 * @see #DpRt_JNI_Error_Number
 * @see #DpRt_JNI_Error_String
 */
void DpRt_JNI_Throw_Exception_String(JNIEnv *env,char *function_name,int error_number,char *error_string)
{
	jclass exception_class = NULL;
	jobject exception_instance = NULL;
	jstring error_jstring = NULL;
	jstring dprt_library_error_jstring = NULL;
	jmethodID mid;
	int retval;

	exception_class = (*env)->FindClass(env,"ngat/dprt/DpRtLibraryNativeException");
	if(exception_class == NULL)
	{
		fprintf(stderr,"DpRt_JNI_Throw_Exception_String:FindClass failed:%s:%d:%s\n",function_name,
			error_number,error_string);
		return;
	}
/* get ngat.dprt.DpRtLibraryNativeException(int errorNumber,String errorString) constructor */
	mid = (*env)->GetMethodID(env,exception_class,"<init>","(ILjava/lang/String;ILjava/lang/String;)V");
	if(mid == 0)
	{
		/* One of the following exceptions has been thrown:
		** NoSuchMethodError, ExceptionInInitializerError, OutOfMemoryError */
		fprintf(stderr,"DpRt_JNI_Throw_Exception_String:GetMethodID failed:%s:%s\n",function_name,
			error_string);
		return;
	}
/* convert error_string to JString */
	error_jstring = (*env)->NewStringUTF(env,error_string);
	dprt_library_error_jstring = (*env)->NewStringUTF(env,DpRt_JNI_Error_String);
/* call constructor */
	exception_instance = (*env)->NewObject(env,exception_class,mid,(jint)DpRt_JNI_Error_Number,
					       dprt_library_error_jstring,
					       (jint)error_number,error_jstring);
	if(exception_instance == NULL)
	{
		/* One of the following exceptions has been thrown:
		** InstantiationException, OutOfMemoryError */
		fprintf(stderr,"DpRt_JNI_Throw_Exception_String:NewObject failed %s:%d:%s:%d:%s\n",
			function_name,DpRt_JNI_Error_Number,DpRt_JNI_Error_String,error_number,error_string);
		return;
	}
/* throw instance */
	retval = (*env)->Throw(env,(jthrowable)exception_instance);
	if(retval !=0)
	{
		fprintf(stderr,"DpRt_JNI_Throw_Exception_String:Throw failed %d:%s:%d:%s:%d:%s\n",retval,
			function_name,DpRt_JNI_Error_Number,DpRt_JNI_Error_String,error_number,error_string);
	}
}

/**
 * libdprt Log Handler for the Java layer interface. This calls the ngat.dprt.ccs.DpRtLibrary logger's 
 * log(int level,String message) method with the parameters supplied to this routine.
 * If the Logger instance is NULL, or the Log_Method_Id is NULL the call is not made.
 * Otherwise, A java.lang.String instance is constructed from the string parameter,
 * and the JNI CallVoidMEthod routine called to call log().
 * @param level The log level of the message.
 * @param string The message to log.
 * @see #Java_VM
 * @see #Logger
 * @see #Log_Method_Id
 */
void DpRt_JNI_Log_Handler(int level,char *string)
{
	JNIEnv *env = NULL;
	jstring java_string = NULL;

	if(Logger == NULL)
	{
		fprintf(stderr,"DpRt_JNI_Log_Handler:Logger was NULL (%d,%s).\n",level,string);
		return;
	}
	if(Log_Method_Id == NULL)
	{
		fprintf(stderr,"DpRt_JNI_Log_Handler:Log_Method_Id was NULL (%d,%s).\n",level,string);
		return;
	}
	if(Java_VM == NULL)
	{
		fprintf(stderr,"DpRt_JNI_Log_Handler:Java_VM was NULL (%d,%s).\n",level,string);
		return;
	}
/* get java env for this thread */
	(*Java_VM)->AttachCurrentThread(Java_VM,(void**)&env,NULL);
	if(env == NULL)
	{
		fprintf(stderr,"DpRt_JNI_Log_Handler:env was NULL (%d,%s).\n",level,string);
		return;
	}
	if(string == NULL)
	{
		fprintf(stderr,"DpRt_JNI_Log_Handler:string (%d) was NULL.\n",level);
		return;
	}
/* convert C to Java String */
	java_string = (*env)->NewStringUTF(env,string);
/* call log method on logger instance */
	(*env)->CallVoidMethod(env,Logger,Log_Method_Id,(jint)level,java_string);
}

/**
 * A routine to return the current value of the error number. The error number is usually 0 for success,
 * and non-zero when an error occurs.
 * @return The current value of the error number.
 * @see #DpRt_JNI_Error_Number
 */
int DpRt_JNI_Get_Error_Number(void)
{
	return DpRt_JNI_Error_Number;
}

/**
 * A routine to get the current value of the error string. The current value is strcpyed into the 
 * supplied paramater. The error string is usually blank when no errors have occured, and non-blank when
 * an error has occured.
 * @param error_string A pointer to an area of memory reserved to accept the error string. This area of 
 * memory should be at least <a href="#DPRT_ERROR_STRING_LENGTH">DPRT_ERROR_STRING_LENGTH</a> in length.
 * @see #DpRt_JNI_Error_String
 */
void DpRt_JNI_Get_Error_String(char *error_string)
{
	strcpy(error_string,DpRt_JNI_Error_String);
}

/* ------------------------------------------------------- */
/* internal functions */
/* ------------------------------------------------------- */
/**
 * Routine to get the value of the keyword from the property file.
 * This routine assumes keyword and value_string have been checked as being non-null.
 * @param keyword The keyword in the property file to look up.
 * @param value_string The address of a pointer to allocate and store the resulting value string in.
 * 	This pointer is dynamically allocated and must be freed using <b>free()</b>. 
 * @see #PROPERTY_FILE_NAME
 */
static int DpRt_JNI_Get_Property_From_C_File(char *keyword,char **value_string)
{
	char buff[256];
	FILE *fp = NULL;
	char *ch = NULL;
	int done;

	fp = fopen(PROPERTY_FILE_NAME,"r");
	if(fp == NULL)
	{
		DpRt_JNI_Error_Number = 17;
		sprintf(DpRt_JNI_Error_String,"DpRt_Get_Property_From_C_File failed: File open (%s) failed.\n",
			PROPERTY_FILE_NAME);
		return FALSE;
	}
	done = FALSE;
	(*value_string) = NULL;
	while((done == FALSE)&&(fgets(buff,255,fp) != NULL))
	{
		if(strncmp(keyword,buff,strlen(keyword))==0)
		{
			ch = strchr(buff,'=');
			if(ch != NULL)
			{
				(*value_string) = (char*)malloc((strlen(ch)+1)*sizeof(char));
				if((*value_string) == NULL)
				{
					fclose(fp);
					DpRt_JNI_Error_Number = 18;
					sprintf(DpRt_JNI_Error_String,"DpRt_Get_Property_From_C_File failed: "
						"Memory Allocation error(%s,%s,%s,%d) failed.\n",
						PROPERTY_FILE_NAME,keyword,ch,(strlen(ch)+1));
					return FALSE;
				}
				strcpy((*value_string),ch+1);
			/* if the string terminates in a new-line, remove it */
				if((strlen((*value_string))>0) && ((*value_string)[strlen((*value_string))-1] == '\n'))
					(*value_string)[strlen((*value_string))-1] = '\0';
				done = TRUE;
			}
		}
	}
	fclose(fp);
	if(done == FALSE)
	{
		DpRt_JNI_Error_Number = 19;
		sprintf(DpRt_JNI_Error_String,"DpRt_Get_Property_From_C_File failed:Failed to find keyword (%s,%s).\n",
			PROPERTY_FILE_NAME,keyword);
	}
	return done;
}

/**
 * Routine to get the integer value of the keyword from the property file.
 * This routine assumes keyword and value have been checked as being non-null.
 * DpRt_Get_Property_From_C_File is used to get the keyword's value, and sscanf used to convert it to
 * an integer.
 * @param keyword The keyword in the property file to look up.
 * @param value_string The address of an integer to store the resulting value string in.
 * @see #DpRt_JNI_Get_Property_From_C_File
 */
static int DpRt_JNI_Get_Property_Integer_From_C_File(char *keyword,int *value)
{
	char *value_string = NULL;
	int retval;

	if(!DpRt_JNI_Get_Property_From_C_File(keyword,&value_string))
		return FALSE;
	retval = sscanf(value_string,"%i",value);
	if(retval != 1)
	{
		DpRt_JNI_Error_Number = 20;
		sprintf(DpRt_JNI_Error_String,"DpRt_Get_Property_Integer_From_C_File failed:"
			"Failed to convert (%s,%s,%s).\n",PROPERTY_FILE_NAME,keyword,value_string);
		if(value_string != NULL)
			free(value_string);
		return FALSE;
	}
	if(value_string != NULL)
		free(value_string);
	return TRUE;
}

/**
 * Routine to get the double value of the keyword from the property file.
 * This routine assumes keyword and value have been checked as being non-null.
 * DpRt_Get_Property_From_C_File is used to get the keyword's value, and sscanf used to convert it to
 * a double.
 * @param keyword The keyword in the property file to look up.
 * @param value_string The address of an double to store the resulting value string in.
 * @see #DpRt_JNI_Get_Property_From_C_File
 */
static int DpRt_JNI_Get_Property_Double_From_C_File(char *keyword,double *value)
{
	char *value_string = NULL;
	int retval;

	if(!DpRt_JNI_Get_Property_From_C_File(keyword,&value_string))
		return FALSE;
	retval = sscanf(value_string,"%lf",value);
	if(retval != 1)
	{
		DpRt_JNI_Error_Number = 21;
		sprintf(DpRt_JNI_Error_String,"DpRt_Get_Property_Double_From_C_File failed:"
			"Failed to convert (%s,%s,%s).\n",PROPERTY_FILE_NAME,keyword,value_string);
		if(value_string != NULL)
			free(value_string);
		return FALSE;
	}
	if(value_string != NULL)
		free(value_string);
	return TRUE;
}

/**
 * Routine to get the boolean value of the keyword from the property file.
 * This routine assumes keyword and value have been checked as being non-null.
 * DpRt_Get_Property_From_C_File is used to get the keyword's value, and the string checked to see if it
 * contains <b>true</b> or <b>false</b>.
 * @param keyword The keyword in the property file to look up.
 * @param value_string The address of an integer to store the resulting value, either TRUE (1) or FALSE (0).
 * @see #DpRt_JNI_Get_Property_From_C_File
 */
static int DpRt_JNI_Get_Property_Boolean_From_C_File(char *keyword,int *value)
{
	char *value_string = NULL;

	if(!DpRt_JNI_Get_Property_From_C_File(keyword,&value_string))
		return FALSE;
	if((strcmp(value_string,"true")==0)||(strcmp(value_string,"TRUE")==0)||(strcmp(value_string,"True")==0))
		(*value) = TRUE;
	else if((strcmp(value_string,"false")==0)||(strcmp(value_string,"FALSE")==0)||
		(strcmp(value_string,"False")==0))
		(*value) = FALSE;
	else
	{
		DpRt_JNI_Error_Number = 22;
		sprintf(DpRt_JNI_Error_String,"DpRt_Get_Property_Boolean_From_C_File failed:"
			"Failed to convert (%s,%s,%s).\n",PROPERTY_FILE_NAME,keyword,value_string);
		if(value_string != NULL)
			free(value_string);
		return FALSE;
	}
	if(value_string != NULL)
		free(value_string);
	return TRUE;
}

/*
** $Log: not supported by cvs2svn $
*/
