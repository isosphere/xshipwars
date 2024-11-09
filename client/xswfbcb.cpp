/*
                     XSW's File Browser Callback Handler

	Functions:

	void XSWMapFB(char *path, int op_code)

        int XSWFBCBOk(char *path)
        int XSWFBCBCancel(char *path)

	---

	Handles mapping and callbacks of file browser.

	Note: Options window file browser mapping is done in
	the optwinop.c module.

 */

#include "starchartwin.h"
#include "xsw.h"



/*
 *	Maps the file browser and sets pri_fb_loadop to the specified
 *	operation code.
 */
void XSWMapFB(char *path, int op_code)
{
	char *strptr;
	char tmp_path[PATH_MAX + NAME_MAX];

	static int prev_fb_loadop = PRI_FB_LOADOP_NONE;


	switch(op_code)
	{
	  /* ***************************************************** */
	  /* Run server. */
	  case PRI_FB_LOADOP_RUN_SERVER:
            XSWDoUnfocusAllWindows();

	    strptr = PrefixPaths(DEF_SW_SERVER_DIR, "*");
	    strncpy(
		tmp_path,
		((strptr == NULL) ? "/" : strptr),
		PATH_MAX + NAME_MAX
	    );
	    tmp_path[PATH_MAX + NAME_MAX - 1] = '\0';

            pri_fb_loadop = op_code;
            pri_fbrowser.func_ok = XSWFBCBOk;
            pri_fbrowser.func_cancel = XSWFBCBCancel;

	    FBrowserSetOpMesg(
		&pri_fbrowser,
		"Start Server",
		"Execute"
	    );
	    FBrowserMapPath(&pri_fbrowser, tmp_path);
            break;

          /* ***************************************************** */
          /* Load Image Set Referance file. */  
          case PRI_FB_LOADOP_ISREF:
            XSWDoUnfocusAllWindows();

            strptr = PrefixPaths(dname.images, FN_ISREF_EXT_MASK);
            strncpy(
                tmp_path,
                ((strptr == NULL) ? "/" : strptr),
                PATH_MAX + NAME_MAX
            );
            tmp_path[PATH_MAX + NAME_MAX - 1] = '\0';

            pri_fb_loadop = op_code;
            pri_fbrowser.func_ok = XSWFBCBOk;
            pri_fbrowser.func_cancel = XSWFBCBCancel;

            FBrowserSetOpMesg(
                &pri_fbrowser,
                "Load Image Set Referance File",
                "Load"
            );
            FBrowserMapPath(&pri_fbrowser, tmp_path);
            break;

          /* ***************************************************** */
          /* Load OCSN file. */
          case PRI_FB_LOADOP_OCSN:
            XSWDoUnfocusAllWindows();

            strptr = PrefixPaths(dname.etc, FN_OCSN_EXT_MASK);
            strncpy(
                tmp_path,
                ((strptr == NULL) ? "/" : strptr),
                PATH_MAX + NAME_MAX
            );
            tmp_path[PATH_MAX + NAME_MAX - 1] = '\0';

            pri_fb_loadop = op_code;
            pri_fbrowser.func_ok = XSWFBCBOk;  
            pri_fbrowser.func_cancel = XSWFBCBCancel;

            FBrowserSetOpMesg(
		&pri_fbrowser,
		"Load OCS Names File",
		"Load"
	    );
            FBrowserMapPath(&pri_fbrowser, tmp_path);
            break;

          /* ***************************************************** */
          /* Load Sound Scheme file */
          case PRI_FB_LOADOP_SS:
            XSWDoUnfocusAllWindows(); 

            strptr = PrefixPaths(dname.sounds, FN_SS_EXT_MASK);
            strncpy(
                tmp_path,
                ((strptr == NULL) ? "/" : strptr),
                PATH_MAX + NAME_MAX
            );
            tmp_path[PATH_MAX + NAME_MAX - 1] = '\0';

            pri_fb_loadop = op_code;
            pri_fbrowser.func_ok = XSWFBCBOk;
            pri_fbrowser.func_cancel = XSWFBCBCancel;

            FBrowserSetOpMesg(
		&pri_fbrowser,
                "Load Sound Scheme File",
		"Load"
	    );
            FBrowserMapPath(&pri_fbrowser, tmp_path);
            break;

          /* ***************************************************** */
          /* Server script. */
          case PRI_FB_LOADOP_SERVER_SCRIPT:
            XSWDoUnfocusAllWindows();

            pri_fb_loadop = op_code;
            pri_fbrowser.func_ok = XSWFBCBOk;
            pri_fbrowser.func_cancel = XSWFBCBCancel;

            FBrowserSetOpMesg(
		&pri_fbrowser,
                "Load Server Script File",
		"Load"
	    );

            /* Previous load operation the same? */
            if(prev_fb_loadop == PRI_FB_LOADOP_SERVER_SCRIPT)
	    {
		FBrowserMapSearchMask(&pri_fbrowser, "*");
	    }
	    else
            {
                strptr = PrefixPaths(
		    getenv("HOME"),
		    "*"
		);
                strncpy(
                    tmp_path,
                    ((strptr == NULL) ? "/" : strptr),
                    PATH_MAX + NAME_MAX
                );
                tmp_path[PATH_MAX + NAME_MAX - 1] = '\0';

                FBrowserMapPath(&pri_fbrowser, tmp_path);
            }
            break;

          /* ***************************************************** */
          /* Starchart. */
          case PRI_FB_LOADOP_STARCHART_OVERLAY:
            XSWDoUnfocusAllWindows();

            pri_fb_loadop = op_code;
            pri_fbrowser.func_ok = XSWFBCBOk;
            pri_fbrowser.func_cancel = XSWFBCBCancel;

            FBrowserSetOpMesg(
                &pri_fbrowser,
                "Load Star Chart",
                "Load"
            );

            /* Previous load operation the same? */
            if((prev_fb_loadop == PRI_FB_LOADOP_STARCHART_OVERLAY) ||
               (prev_fb_loadop == PRI_FB_SAVEOP_STARCHART)
	    )
            {
                FBrowserMapSearchMask(&pri_fbrowser, FN_STARCHART_EXT_MASK);
            }
            else
            {
                strncpy(
                    tmp_path, 
                    PrefixPaths(dname.starchart, FN_STARCHART_EXT_MASK),
                    PATH_MAX + NAME_MAX
                );
                tmp_path[PATH_MAX + NAME_MAX - 1] = '\0';

                FBrowserMapPath(&pri_fbrowser, tmp_path);
            }
	    break;

          case PRI_FB_SAVEOP_STARCHART:
            XSWDoUnfocusAllWindows();

            pri_fb_loadop = op_code;
            pri_fbrowser.func_ok = XSWFBCBOk;
            pri_fbrowser.func_cancel = XSWFBCBCancel;

            FBrowserSetOpMesg(
                &pri_fbrowser,
                "Save Star Chart",
                "Save"
            );
          
            /* Previous load operation the same? */
            if((prev_fb_loadop == PRI_FB_LOADOP_STARCHART_OVERLAY) ||
               (prev_fb_loadop == PRI_FB_SAVEOP_STARCHART)
            ) 
            {
                FBrowserMapSearchMask(&pri_fbrowser, FN_STARCHART_EXT_MASK);
            }
            else  
            {
		strncpy(
		    tmp_path,
		    PrefixPaths(dname.starchart, FN_STARCHART_EXT_MASK),
		    PATH_MAX + NAME_MAX
		);
		tmp_path[PATH_MAX + NAME_MAX - 1] = '\0';

                FBrowserMapPath(&pri_fbrowser, tmp_path);
            }
            break;

          /* ***************************************************** */
	  default:
	    fprintf(
		stderr,
		"XSWMapFB(): Unknown operation code `%i'.\n",
		op_code
	    );
	    break;
	}


	/* Record previous load operation. */
	prev_fb_loadop = op_code;


	return;
}


/*
 *	Primary file browser ok callback handler.
 */
int XSWFBCBOk(char *path)
{
        if(path != NULL)
        {
	    char *strptr;
	    char tmp_path[PATH_MAX + NAME_MAX];

	    switch(pri_fb_loadop)
            {
              case PRI_FB_LOADOP_RUN_SERVER:
                XSWStartServer(path);
                break;

              case PRI_FB_LOADOP_OCSN:
	        XSWLoadOCSN(path);
		break;

	      case PRI_FB_LOADOP_ISREF:
		XSWLoadIsrefs(path);
		break;

	      case PRI_FB_LOADOP_SS:
		XSWLoadSS(path);
		break;

	      case PRI_FB_LOADOP_SERVER_SCRIPT:
		ServScriptDoMapPrompt(path);
		break;

	      case PRI_FB_LOADOP_STARCHART_OVERLAY:
		/* Record parent path. */
		strptr = GetParentDir(path);
		if(strptr != NULL)
		{
		    strncpy(tmp_path, strptr, PATH_MAX + NAME_MAX);
		    tmp_path[PATH_MAX + NAME_MAX - 1] = '\0';

		    if(ISPATHDIR(tmp_path))
		    {
			strncpy(
			    dname.starchart,
			    tmp_path,
			    PATH_MAX
			);
			dname.starchart[PATH_MAX - 1] = '\0';
		    }
		}

		SChtOverlayChartCB(&starchart_win, path);
		break;

	      case PRI_FB_SAVEOP_STARCHART:
                /* Record parent path. */
                strptr = GetParentDir(path);
                if(strptr != NULL)
                {
                    strncpy(tmp_path, strptr, PATH_MAX + NAME_MAX);
                    tmp_path[PATH_MAX + NAME_MAX - 1] = '\0';

                    if(ISPATHDIR(tmp_path))
                    {
                        strncpy(
                            dname.starchart,
                            tmp_path,
                            PATH_MAX
                        );
                        dname.starchart[PATH_MAX - 1] = '\0';
                    }
                }

		SChtSaveChartCB(&starchart_win, path);
		break;

	      default:
		break;
	    }
        }  


        /* Reset global file browser operation code. */
        pri_fb_loadop = PRI_FB_LOADOP_NONE;


        return(0);
}

/*
 *	Primary file browser cancel callback handler.
 */
int XSWFBCBCancel(char *path)
{
        /* Set global pri_fb_loadop to load operation. */
        pri_fb_loadop = PRI_FB_LOADOP_NONE;

	return(0);
}
