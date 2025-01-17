//------------------------------------------------------------------------------
// CCOLAMD/MATLAB/ccolamdmex.c: MATLAB interface for CCOLAMD
//------------------------------------------------------------------------------

// CCOLAMD, Copyright (c) 2005-2022, Univ. of Florida, All Rights Reserved.
// Authors: Timothy A. Davis, Sivasankaran Rajamanickam, and Stefan Larimore.
// SPDX-License-Identifier: BSD-3-clause

//------------------------------------------------------------------------------

/* 
 * Usage:
 *	p = ccolamd (A) ;
 *	[p stats] = ccolamd (A, knobs, cmember) ;
 *
 * See ccolamd.m for a description.
 */

/* ========================================================================== */
/* === Include files ======================================================== */
/* ========================================================================== */

#include "ccolamd.h"
#include "mex.h"
#include "matrix.h"
#include <stdlib.h>
#include <string.h>

/* ========================================================================== */
/* === ccolamd mexFunction ================================================== */
/* ========================================================================== */

void mexFunction
(
    /* === Parameters ======================================================= */

    int nargout,
    mxArray *pargout [ ],
    int nargin,
    const mxArray *pargin [ ]
)
{
    /* === Local variables ================================================== */

    int64_t *A ;            /* ccolamd's copy of the matrix and workspace */
    int64_t *cmember ;      /* ccolamd's copy of the constraint set */
    double *in_cmember ;    /* input constraint set */
    int64_t *p ;            /* ccolamd's copy of the column pointers */
    int64_t Alen ;          /* size of A */
    int64_t cslen ;         /* size of CS  */
    int64_t n_col ;         /* number of columns of A */
    int64_t n_row ;         /* number of rows of A */
    int64_t nnz ;           /* number of entries in A */
    int full ;              /* TRUE if input matrix full, FALSE if sparse */
    double knobs [CCOLAMD_KNOBS] ; /* ccolamd user-controllable parameters */
    double *out_perm ;      /* output permutation vector */
    double *out_stats ;     /* output stats vector */
    double *in_knobs ;      /* input knobs vector */
    int64_t i ;             /* loop counter */
    mxArray *Ainput ;       /* input matrix handle */
    int spumoni ;           /* verbosity variable */
    int64_t stats [CCOLAMD_STATS] ; /* stats for ccolamd */

    /* === Check inputs ===================================================== */

    if (nargin < 1 || nargin > 3 || nargout < 0 || nargout > 2)
    {
	mexErrMsgTxt ("Usage: [p stats] = ccolamd (A, knobs, cmember)") ;
    }

    /* === Get cmember ====================================================== */

    cmember = NULL ;
    cslen = 0 ;
    if (nargin > 2)
    {
	in_cmember = mxGetPr (pargin [2]) ;
	cslen = mxGetNumberOfElements (pargin [2]) ;
	if (cslen != 0)
	{
	    cmember = (int64_t *) mxCalloc (cslen, sizeof (int64_t)) ;
	    for (i = 0 ; i < cslen ; i++)
	    {
		/* convert cmember from 1-based to 0-based */
		cmember[i] = ((int64_t) in_cmember [i] - 1) ;
	    }
	}
    }

    /* === Get knobs ======================================================== */

    ccolamd_l_set_defaults (knobs) ;
    spumoni = 0 ;

    /* check for user-passed knobs */
    if (nargin > 1)
    {
	in_knobs = mxGetPr (pargin [1]) ;
	i = mxGetNumberOfElements (pargin [1]) ;
	if (i > 0) knobs [CCOLAMD_LU] = (in_knobs [0] != 0) ;
	if (i > 1) knobs [CCOLAMD_DENSE_ROW]  = in_knobs [1] ;
	if (i > 2) knobs [CCOLAMD_DENSE_COL]  = in_knobs [2] ;
	if (i > 3) knobs [CCOLAMD_AGGRESSIVE] = (in_knobs [3] != 0) ;
	if (i > 4) spumoni = (in_knobs [4] != 0) ;
    }

    /* print knob settings if spumoni is set */
    if (spumoni)
    {
	mexPrintf ("\nccolamd version %d.%d, %s:\nknobs(1): %g, order for %s\n",
	    CCOLAMD_MAIN_VERSION, CCOLAMD_SUB_VERSION, CCOLAMD_DATE,
	    in_knobs [0],
	    (knobs [CCOLAMD_LU] != 0) ? "lu(A)" : "chol(A'*A)") ;
	if (knobs [CCOLAMD_DENSE_ROW] >= 0)
	{
	    mexPrintf ("knobs(2): %g, rows with > max(16,%g*sqrt(size(A,2)))"
		" entries removed\n", in_knobs [1], knobs [CCOLAMD_DENSE_ROW]) ;
	}
	else
	{
	    mexPrintf ("knobs(2): %g, no dense rows removed\n", in_knobs [1]) ;
	}
	if (knobs [CCOLAMD_DENSE_COL] >= 0)
	{
	    mexPrintf ("knobs(3): %g, cols with > max(16,%g*sqrt(min(size(A)))"
		" entries removed\n", in_knobs [2], knobs [CCOLAMD_DENSE_COL]) ;
	}
	else
	{
	    mexPrintf ("knobs(3): no dense columns removed\n", in_knobs [2]) ;
	}
	mexPrintf ("knobs(4): %g, aggressive absorption: %s\n",
	    in_knobs [3], (knobs [CCOLAMD_AGGRESSIVE] != 0) ? "yes" : "no") ;
	mexPrintf ("knobs(5): %g, statistics and knobs printed\n",
	    in_knobs [4]) ;
    }

    /* === If A is full, convert to a sparse matrix ========================= */

    Ainput = (mxArray *) pargin [0] ;
    if (mxGetNumberOfDimensions (Ainput) != 2)
    {
	mexErrMsgTxt ("ccolamd: input matrix must be 2-dimensional") ;
    }
    full = !mxIsSparse (Ainput) ;
    if (full)
    {
	mexCallMATLAB (1, &Ainput, 1, (mxArray **) pargin, "sparse") ;
    }

    /* === Allocate workspace for ccolamd =================================== */

    /* get size of matrix */
    n_row = mxGetM (Ainput) ;
    n_col = mxGetN (Ainput) ;

    /* get column pointer vector */
    p = (int64_t *) mxCalloc (n_col+1, sizeof (int64_t)) ;
    (void) memcpy (p, mxGetJc (Ainput), (n_col+1)*sizeof (int64_t)) ;
    nnz = p [n_col] ;
    Alen = (int64_t) ccolamd_l_recommended (nnz, n_row, n_col) ;
    if (Alen == 0)
    {
    	mexErrMsgTxt ("ccolamd: problem too large") ;
    }

    /* === Copy input matrix into workspace ================================= */

    A = (int64_t *) mxCalloc (Alen, sizeof (int64_t)) ;
    (void) memcpy (A, mxGetIr (Ainput), nnz*sizeof (int64_t)) ;

    if (full)
    {
	mxDestroyArray (Ainput) ;
    }

    /* Check constraint set size */
    if (cmember != NULL && cslen != n_col)
    {
    	mexErrMsgTxt ("ccolamd: cmember must be of length equal to #cols of A");
    }

    /* === Order the columns (destroys A) =================================== */

    if (!ccolamd_l (n_row, n_col, Alen, A, p, knobs, stats, cmember))
    {
	ccolamd_l_report (stats) ;
	mexErrMsgTxt ("ccolamd error!") ;
    }
    mxFree (A) ;
    mxFree (cmember) ;

    /* === Return the permutation vector ==================================== */

    pargout [0] = mxCreateDoubleMatrix (1, n_col, mxREAL) ;
    out_perm = mxGetPr (pargout [0]) ;
    for (i = 0 ; i < n_col ; i++)
    {
	/* ccolamd is 0-based, but MATLAB expects this to be 1-based */
	out_perm [i] = p [i] + 1 ;
    }
    mxFree (p) ;

    /* === Return the stats vector ========================================== */

    /* print stats if spumoni is set */
    if (spumoni)
    {
	ccolamd_l_report (stats) ;
    }

    if (nargout == 2)
    {
	pargout [1] = mxCreateDoubleMatrix (1, CCOLAMD_STATS, mxREAL) ;
	out_stats = mxGetPr (pargout [1]) ;
	for (i = 0 ; i < CCOLAMD_STATS ; i++)
	{
	    out_stats [i] = stats [i] ;
	}

	/* fix stats (5) and (6), for 1-based information on jumbled matrix. */
	/* note that this correction doesn't occur if symamd returns FALSE */
	out_stats [CCOLAMD_INFO1] ++ ; 
	out_stats [CCOLAMD_INFO2] ++ ; 
    }
}
