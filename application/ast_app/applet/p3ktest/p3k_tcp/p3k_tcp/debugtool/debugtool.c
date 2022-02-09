#include "debugtool.h"


FILE *gs_debugFile = NULL;


/*
    If you want to redirect the output, use the function
    eg: If you want to output the error message to a file, you can reassign a value to the err_file
*/
void  px_dbg_file(FILE *dbgfile) 
{
  
    if (dbgfile != NULL)
        gs_debugFile = dbgfile;

}
void px_dpg_file_clean()
{
	gs_debugFile = NULL;
}

void __DBG_INFO (const char* file,const char*func, int line, const char *format, ...)
{
	va_list ap;
   	va_start (ap, format);

	if (gs_debugFile == NULL)
    	{
   
		fprintf (stdout, " INFO: File[ %s ], Function[ %s ],at Line[ %d ] : ", file, func,line);
		vfprintf (stdout, format, ap);
		fflush(stdout);
		
	 }
	else
	{
   		fprintf (gs_debugFile, " INFO: File[ %s ], Function[ %s ],at Line[ %d ] : ", file, func,line);
   	 	vfprintf (gs_debugFile, format, ap);
   	 	fflush(gs_debugFile);
	}
	va_end (ap);
}
void __DBG_WARN (const char* file,const char*func, int line, const char *format, ...)
{
 	va_list ap;
    	va_start (ap, format);
    if (gs_debugFile == NULL)
      {
	   
	    fprintf (stderr, "\033[33mWARN: File[ %s ], Function[ %s ],at Line[ %d ] : ", file, func,line);
	    vfprintf (stderr, format, ap);
	    fprintf (stderr,"\033[0m");
	    fflush(stderr);
	   


	}  
	else 
	{

		fprintf (gs_debugFile, "WARN: File[ %s ], Function[ %s ],at Line[ %d ] : ", file, func,line);
		vfprintf (gs_debugFile, format, ap);

		fflush(gs_debugFile);
		
	}
	va_end (ap);
}
void __DBG_ERR (const char* file,const char*func, int line, const char *format, ...)
{
	  va_list ap;
	  va_start (ap, format);
    if (gs_debugFile == NULL)
    {
	  
	    fprintf (stderr, "\033[31mERR : File[ %s ], Function[ %s ],at Line[ %d ] : ", file, func,line);
	    vfprintf (stderr, format, ap);
	    fprintf (stderr,"\033[0m");
	    fflush(stderr);

    }
      else
      	{

		fprintf (gs_debugFile, "ERR : File[ %s ], Function[ %s ],at Line[ %d ] : ", file, func,line);
		vfprintf (gs_debugFile, format, ap);
		fflush(gs_debugFile);
		
      	}
	 va_end (ap);
}
