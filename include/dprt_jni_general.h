/*   
    Copyright 2006, Astrophysics Research Institute, Liverpool John Moores University.

    This file is part of DpRt.

    DpRt is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    DpRt is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with DpRt; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/
/* dprt_jni_general.h
** $Header: /home/cjm/cvs/libdprt-jni_general/include/dprt_jni_general.h,v 1.3 2012-11-12 16:23:00 cjm Exp $
*/
#ifndef DPRT_JNI_GENERAL_H
#define DPRT_JNI_GENERAL_H

/* needed for function prototypes */
#include <jni.h>

/**
 * TRUE is the value usually returned from routines to indicate success.
 */
#ifndef TRUE
#define TRUE 1
#endif
/**
 * FALSE is the value usually returned from routines to indicate failure.
 */
#ifndef FALSE
#define FALSE 0
#endif

/**
 * This is the length of error string of modules in the library.
 * @see #DpRt_Error_String
 */
#define DPRT_ERROR_STRING_LENGTH	256

/* variable declarations */
extern int DpRt_JNI_Error_Number;
extern char DpRt_JNI_Error_String[];

/* function declarations */
/* initialisation/finalisation */
extern void DpRt_JNI_Set_Java_VM(JavaVM *vm);
extern void DpRt_JNI_Set_Status(JNIEnv *env,jobject object,jobject status);
extern void DpRt_JNI_Initialise_Logger_Reference(JNIEnv *env,jobject obj,jobject l);
extern void DpRt_JNI_Finalise_Logger_Reference(JNIEnv *env);
extern void DpRt_JNI_Finalise_Status_Reference(JNIEnv *env);
extern int DpRt_JNI_Initialise(void);
/* abort */
extern void DpRt_JNI_Set_Abort(int value);
extern int DpRt_JNI_Get_Abort(void);
/* top level client API for getting property */
extern int DpRt_JNI_Get_Property(char *keyword,char **value_string);
extern int DpRt_JNI_Get_Property_Integer(char *keyword,int *value);
extern int DpRt_JNI_Get_Property_Double(char *keyword,double *value);
extern int DpRt_JNI_Get_Property_Boolean(char *keyword,int *value);
/* routines to set function pointer for property */
extern void DpRt_JNI_Set_Property_Function_Pointer(int (*get_property_fp)(char *keyword,char **value_string));
extern void DpRt_JNI_Set_Property_Integer_Function_Pointer(int (*get_property_integer_fp)(char *keyword,int *value));
extern void DpRt_JNI_Set_Property_Double_Function_Pointer(int (*get_property_double_fp)(char *keyword,double *value));
extern void DpRt_JNI_Set_Property_Boolean_Function_Pointer(int (*get_property_boolean_fp)(char *keyword,int *value));
/* routines to get properties via the DpRtStatus object.
** Should not be used directly, should be parameters to DpRt_JNI_Set_Property_*_Function_Pointer
** and the top-level DpRt_JNI_Get_Property* should be used to actually get property values. */
extern int DpRt_JNI_DpRtStatus_Get_Property(char *keyword,char **value_string);
extern int DpRt_JNI_DpRtStatus_Get_Property_Integer(char *keyword,int *value);
extern int DpRt_JNI_DpRtStatus_Get_Property_Double(char *keyword,double *value);
extern int DpRt_JNI_DpRtStatus_Get_Property_Boolean(char *keyword,int *value);
/* routines to set command dones */
extern int DpRt_JNI_Set_Command_Done(JNIEnv *env,jclass cls,jobject done,
					int successful,int error_number,char *error_string);
extern int DpRt_JNI_Set_Reduce_Done(JNIEnv *env,jclass cls,jobject done,char *output_filename);
extern int DpRt_JNI_Set_Calibrate_Reduce_Done(JNIEnv *env,jclass cls,jobject done,
					      double mean_counts,double peak_counts);
extern int DpRt_JNI_Set_Expose_Reduce_Done(JNIEnv *env,jclass cls,jobject done,double seeing,double counts,
					   double x_pix,double y_pix,double photometricity,
					   double sky_brightness,int saturated);
/* exception handling */
extern void DpRt_JNI_Throw_Exception(JNIEnv *env,char *function_name);
extern void DpRt_JNI_Throw_Exception_String(JNIEnv *env,char *function_name,int error_number,char *error_string);
/* logging back to Java layer */
extern void DpRt_JNI_Log_Handler(char* sub_system,char* source_filename,char* function,int level,char* category,
				 char *string);
/* error retrieval */
extern int DpRt_JNI_Get_Error_Number(void);
extern void DpRt_JNI_Get_Error_String(char *error_string);
#endif
