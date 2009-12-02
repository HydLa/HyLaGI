/****************************************************************************
 * RealPaver v. 1.0                                                         *
 *--------------------------------------------------------------------------*
 * Author: Laurent Granvilliers                                             *
 * Copyright (c) 1999-2003 Institut de Recherche en Informatique de Nantes  *
 * Copyright (c) 2004-2006 Laboratoire d'Informatique de Nantes Atlantique  *
 *--------------------------------------------------------------------------*
 * RealPaver is distributed WITHOUT ANY WARRANTY. Read the associated       *
 * COPYRIGHT file for more details.                                         *
 *--------------------------------------------------------------------------*
 * rp_propagator.cpp                                                        *
 ****************************************************************************/

#include "rp_propagator.h"

/* Creation of an empty dependency function */
void rp_dependency_create(rp_dependency * d)
{
  rp_dependency_size(*d) = 0;
  rp_dependency_ptr(*d) = NULL;
}

/* Destruction of a dependency function */
void rp_dependency_destroy(rp_dependency * d)
{
  int i;
  if (rp_dependency_size(*d)>0)
  {
    for (i=0; i<rp_dependency_size(*d); ++i)
    {
      rp_vector_destroy(&rp_dependency_elem(*d,i));
    }
    rp_free(rp_dependency_ptr(*d));
  }
}

/* Insertion of o in the dependency of v in d */
int rp_dependency_insert(rp_dependency d, rp_operator * o, int v)
{
  /* Dependency for v to be created */
  if (rp_dependency_size(d)<(v+1))
  {
    int i;
    if (rp_dependency_size(d)==0)
    {
      rp_malloc(rp_dependency_ptr(d),rp_vector*,(v+1)*sizeof(rp_vector));
    }
    else
    {
      rp_realloc(rp_dependency_ptr(d),rp_vector*,(v+1)*sizeof(rp_vector));
    }
    for (i=rp_dependency_size(d); i<=v; ++i)
    {
      rp_vector_create_basic(&rp_dependency_elem(d,i));
    }
    rp_dependency_size(d) = v+1;
  }
  return( rp_vector_insert(rp_dependency_elem(d,v),o) );
}

/* Construction of a queue of operators */
void rp_oqueue_create(rp_oqueue * q, int size)
{
  rp_malloc(*q,rp_oqueue,sizeof(rp_oqueue_def));
  rp_oqueue_size(*q) = size;
  if (size==0)
  {
    rp_oqueue_ptr(*q) = NULL;
  }
  else
  {
    rp_malloc(rp_oqueue_ptr(*q),rp_operator**,size*sizeof(rp_operator*));
  }
  rp_oqueue_set_empty(*q);
}

/* Destruction of a queue of operators */
void rp_oqueue_destroy(rp_oqueue * q)
{
  if (rp_oqueue_size(*q)>0)
  {
    rp_free(rp_oqueue_ptr(*q));
  }
  rp_free(*q);
}

/* Enlarge the size of a queue */
void rp_oqueue_enlarge(rp_oqueue * q, int size)
{
  if (size>0)
  {
    rp_oqueue_size(*q) += size;
    rp_realloc(rp_oqueue_ptr(*q),
	       rp_operator**,
	       rp_oqueue_size(*q)*sizeof(rp_operator*));
  }
}

/* Initialization of a queue --> empty */
void rp_oqueue_set_empty(rp_oqueue q)
{
  rp_oqueue_num(q)   = 0;
  rp_oqueue_first(q) =
  rp_oqueue_last(q)  = -1;
}

/* Insertion of an operator in a queue */
void rp_oqueue_push(rp_oqueue q, rp_operator * o)
{
  if (rp_oqueue_num(q)==0)
  {
    rp_oqueue_first(q) = rp_oqueue_last(q) = 0;
  }
  else
  {
    if (rp_oqueue_last(q)==rp_oqueue_size(q)-1)
    {
      rp_oqueue_last(q) = 0;
    }
    else
    {
      ++ rp_oqueue_last(q);
    }
  }
  rp_oqueue_elem(q,rp_oqueue_last(q)) = o;
  ++ rp_oqueue_num(q);
}

/* the top element */
rp_operator * rp_oqueue_pop(rp_oqueue q)
{
  rp_operator * o;
  if (rp_oqueue_empty(q))
  {
    o = NULL;
  }
  else
  {
    o = rp_oqueue_elem(q,rp_oqueue_first(q));
    -- rp_oqueue_num(q);
    if (rp_oqueue_num(q)==0)
    {
      rp_oqueue_set_empty(q);
    }
    else
    {
      if (rp_oqueue_first(q)==rp_oqueue_size(q)-1)
      {
	rp_oqueue_first(q) = 0;
      }
      else
      {
	++ rp_oqueue_first(q);
      }
    }
  }
  return( o );
}

/* Creation of a priority queue */
void rp_oqueue_list_create(rp_oqueue_list * q)
{
  rp_oqueue_list_size(*q) = rp_oqueue_list_num(*q) = 0;
  rp_oqueue_list_ptrp(*q) = NULL;
  rp_oqueue_list_ptrq(*q) = NULL;
}

/* Destruction of a priority queue */
void rp_oqueue_list_destroy(rp_oqueue_list * q)
{
  if (rp_oqueue_list_size(*q)>0)
  {
    int i;
    for (i=0; i<rp_oqueue_list_size(*q); ++i)
    {
      rp_oqueue_destroy(&rp_oqueue_list_elem(*q,i));
    }
    rp_free(rp_oqueue_list_ptrp(*q));
    rp_free(rp_oqueue_list_ptrq(*q));
  }
}

/* q := empty set */
void rp_oqueue_list_set_empty(rp_oqueue_list q)
{
  int i;
  for (i=0; i<rp_oqueue_list_size(q); ++i)
  {
    rp_oqueue_set_empty(rp_oqueue_list_elem(q,i));
  }
  rp_oqueue_list_num(q) = 0;
}

/* Every operator that can be inserted in the queue during propagation */
/* must be declared once by a call to this function                    */
void rp_oqueue_list_insert(rp_oqueue_list q, rp_operator * o)
{
  /* Empty queue */
  if (rp_oqueue_list_size(q)==0)
  {
    rp_oqueue_list_size(q) = 1;

    rp_malloc(rp_oqueue_list_ptrp(q),int*,sizeof(int));
    rp_oqueue_list_priority(q,0) = o->priority();

    rp_malloc(rp_oqueue_list_ptrq(q),rp_oqueue*,sizeof(rp_oqueue));
    rp_oqueue_create(&rp_oqueue_list_elem(q,0),1);
  }
  else
  {
    int i = 0, j;

    /* Check whether q contains a queue associated with the priority of o */
    /* Priorities are sorted in descending ordering                       */
    while ((i<rp_oqueue_list_size(q)) &&
	   (rp_oqueue_list_priority(q,i)>o->priority()))
    {
      ++ i;
    }

    /* No such queue */
    if ((i==rp_oqueue_list_size(q)) ||
	(rp_oqueue_list_priority(q,i)!=o->priority()))
    {
      ++ rp_oqueue_list_size(q);
      rp_realloc(rp_oqueue_list_ptrp(q),
		 int*,
		 rp_oqueue_list_size(q)*sizeof(int));
      rp_realloc(rp_oqueue_list_ptrq(q),
		 rp_oqueue*,
		 rp_oqueue_list_size(q)*sizeof(rp_oqueue));

      /* Management of descending ordering --> creation of a hole at i */
      for (j = rp_oqueue_list_size(q)-1; j>i; --j)
      {
	rp_oqueue_list_priority(q,j) = rp_oqueue_list_priority(q,j-1);
	rp_oqueue_list_elem(q,j) = rp_oqueue_list_elem(q,j-1);
      }

      /* Creation of the new queue */
      rp_oqueue_list_priority(q,i) = o->priority();
      rp_oqueue_create(&rp_oqueue_list_elem(q,i),1);
    }
    else
    {
      rp_oqueue_enlarge(&rp_oqueue_list_elem(q,i),1);
    }
  }
}

/* Push a working operator in the priority queue during propagation */
void rp_oqueue_list_push(rp_oqueue_list q, rp_operator * o)
{
  /* Push in the queue of corresponding priority */
  int i = 0;
  while (rp_oqueue_list_priority(q,i)>o->priority())
  {
    ++ i;
  }
  ++ rp_oqueue_list_num(q);
  rp_oqueue_push(rp_oqueue_list_elem(q,i),o);
}

/* Pop a working operator from the priority queue during propagation */
rp_operator * rp_oqueue_list_pop (rp_oqueue_list q)
{
  /* Find the non empty queue having a maximal priority */
  int i = 0;
  while (rp_oqueue_empty(rp_oqueue_list_elem(q,i)))
  {
    ++ i;
  }
  -- rp_oqueue_list_num(q);
  return( rp_oqueue_pop(rp_oqueue_list_elem(q,i)) );
}

// Constructor
rp_propagator::rp_propagator(rp_problem * p, double improve):
  rp_operator(0,0,0),
  _problem(p),
  _id(RP_OPERATOR_WORKING_INIT),
  _improve(improve),
  _priority(0)
{
  rp_vector_create_basic(&_vop);
  rp_dependency_create(&_dep);
  rp_oqueue_list_create(&_queue);
  rp_box_create(&_bsave,0);
  rp_intset_create(&_vars);
  rp_intset_create(&_pruned_vars);
}

// Destructor
rp_propagator::~rp_propagator()
{
  for (int i=0; i<rp_vector_size(_vop); ++i)
  {
    rp_operator * o = (rp_operator*)rp_vector_elem(_vop,i);
    rp_delete(o);
  }
  rp_oqueue_list_destroy(&_queue);
  rp_dependency_destroy(&_dep);
  rp_vector_destroy(&_vop);
  rp_box_destroy(&_bsave);
  rp_intset_destroy(&_vars);
  rp_intset_destroy(&_pruned_vars);
}

// Operator virtual functions
int rp_propagator::priority() const
{
  return( _priority );
}

int rp_propagator::arity() const
{
  return( rp_intset_size(_vars) );
}

int rp_propagator::var(int i) const
{
  return( rp_intset_elem(_vars,i) );
}

int rp_propagator::pruned_arity() const
{
  return( rp_intset_size(_pruned_vars) );
}

int rp_propagator::pruned_var(int i) const
{
  return( rp_intset_elem(_pruned_vars,i) );
}

// Accessors and modifiers
double rp_propagator::improve() const
{
  return( _improve );
}
void rp_propagator::set_improve(double x)
{
  _improve = x;
}

// Insertion of an operator
void rp_propagator::insert(rp_operator * o)
{
  rp_vector_insert(_vop,o);

  /* Insertion of o in the dependency of all its variables */
  for (int i=0; i<o->arity(); ++i)
  {
    rp_dependency_insert(_dep,o,o->var(i));
  }

  /* Modification of the priority queue */
  rp_oqueue_list_insert(_queue,o);

  /* Modification of priority */
  _priority += o->priority();

  /* Modification of sets of variables */
  for (int i=0; i<o->arity(); ++i)
  {
    rp_intset_insert(_vars,o->var(i));
  }
  for (int i=0; i<o->pruned_arity(); ++i)
  {
    rp_intset_insert(_pruned_vars,o->pruned_var(i));
  }
}

// Checks if the operator can be applied
int rp_propagator::check_precision(rp_operator * o, rp_box b)
{
  return( 1 );

  /* Dangerous to stop the application of one operator
     but it can be efficient for slow convergences...


  for (int i=0; i<o->pruned_arity(); ++i)
  {
    int v = o->pruned_var(i);
    double eps = rp_min_num(1.0e-14,
			    rp_variable_precision(rp_problem_var(*_problem,v)));
    if (rp_interval_width(rp_box_elem(b,v))>(eps))
    {
      return( 1 );
    }
  }
  return( 0 );
  */
}

// Application once the working operators have been defined
int rp_propagator::apply_loop(rp_box b)
{
  // Enlarge the size of _bsave if necessary
  if (rp_box_size(_bsave)<rp_box_size(b))
  {
    rp_box_enlarge_size(&_bsave,rp_box_size(b)-rp_box_size(_bsave));
  }

  // Loop until empty queue or empty domain
  while (!rp_oqueue_list_empty(_queue))
  {
    rp_operator * o = rp_oqueue_list_pop(_queue);
    rp_box_copy(_bsave,b);
    o->set_unworking();  // o is no longer in the list

    if (this->check_precision(o,b))
    {
      if (o->apply(b))
      {
	// Propagation for every variable that can be modified by o
	for (int i=0; i<o->pruned_arity(); ++i)
	{
	  // Consideration of all the operators depending on a modified variable
	  int v = o->pruned_var(i);

	  // Integer variables : correction Nicolas
	  if(rp_variable_integer(rp_problem_var(*_problem,v)))
	  {
	    rp_interval_trunc(rp_box_elem(b,v));

	    if (rp_interval_empty(rp_box_elem(b,v)))
	    {
	      rp_box_set_empty(b);
	      return( 0 );
	    }
	  }

	  // Propagation only if domain improved enough
	  if (rp_interval_improved(rp_box_elem(b,v),
				   rp_box_elem(_bsave,v),
				   _improve))
	  {
	    rp_vector * depv = &rp_dependency_elem(_dep,v);
	    for (int j=0; j<rp_vector_size(*depv); ++j)
	    {
	      rp_operator * odep = (rp_operator *)rp_vector_elem(*depv,j);
	      if (!odep->working(_id))
	      {
		odep->set_working(_id);
		rp_oqueue_list_push(_queue,odep);
	      }
	    }
	  }
	}
	// Note: o is supposed not to be idempotent and then it is necessarily
	// inserted in the queue if the box is modified since it belongs to
	// the dependency of every modified variable
      }
      else
      {
	rp_box_set_empty(b);
	return( 0 );
      }
    }
  }
  return( 1 );
}

// Reduction of b using all the operators
// Useful for the first propagation process
int rp_propagator::apply(rp_box b)
{
  ++_id;
  
  // Set the operators that must be applied
  rp_oqueue_list_set_empty(_queue);
  for (int i=0; i<rp_vector_size(_vop); ++i)
  {
    rp_operator * o = (rp_operator*)rp_vector_elem(_vop,i);
    o->set_working(_id);
    rp_oqueue_list_push(_queue,o);
  }
  
  // Application
  return( apply_loop(b) );
}

// Reduction of b initially using only the operators depending on v
// Useful during search when only one variable is split
int rp_propagator::apply(rp_box b, int v)
{
  ++_id;

  // Set the operators that must be applied
  rp_oqueue_list_set_empty(_queue);
  for (int i=0; i<rp_vector_size(rp_dependency_elem(_dep,v)); ++i)
  {
    rp_operator * o = (rp_operator*)rp_vector_elem(rp_dependency_elem(_dep,v),i);
    o->set_working(_id);
    rp_oqueue_list_push(_queue,o);
  }

  // Application
  return( apply_loop(b) );
}

// Copy protection
rp_propagator::rp_propagator(const rp_propagator& p):
  rp_operator(p)
{
  // --> nothing to do
}

// Copy protection
rp_propagator&
rp_propagator::operator=(const rp_propagator& p)
{
  // --> nothing to do
  return( *this );
}
