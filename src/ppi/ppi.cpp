// Bestimmung der Primitiven Partitionsidentit"aten (PPI) nach [Urbaniak];
// Verwaltung der Testvektoren mit `Digital trees'.

// $Id$

#include <stdio.h>
#include <bool.h>
#include <set>
#include <vector>
#include <iostream.h>
#include <iomanip.h>
#include "vec.h"

#define TALKATIVE 0

typedef set<Vector, less<Vector> > SimpleVectorSet;
static vector<Vector *> VectorRepository;

Vector Vector::operator-() const 
  return v; 
{
  v = Vector(size());
  for (int i = 0; i<size(); i++) v[i] = -(*this)[i];
}

// Implementation with digital trees (FIXME: name?)

class Node {
public:
  bool isleaf;
  Node(bool leaf) : isleaf(leaf) {}
};

class InnerNode : public Node {
public:
  vector<Node*> Children;
  InnerNode(int size) : Node(false), Children(size) {}
};

class LeafNode : public Node {
public:
  Leaf leaf;
  LeafNode() : Node(true) {}
};  

class DigitalTree {
  InnerNode *root;
  bool DoSearch(Leaf min, Leaf max, Report report, int level, 
		Node *node);
  void Destroy(Node *node);
  void DestructiveStore(Node *node, SimpleVectorSet &S);
public:
  int Dimension;
  DigitalTree(int dimension) : 
    Dimension(dimension), 
    root(new InnerNode(2*dimension+1)) {}
  ~DigitalTree() { if (root) Destroy(root); }
  bool Insert(Leaf leaf, bool CheckDup);
  bool OrthogonalRangeSearch(Leaf min, Leaf max, 
			     Report report)
    { return DoSearch(min, max, report, Dimension-1, root); }
  void DestructiveCopy(SimpleVectorSet &S)
    { DestructiveStore(root, S); root = 0; }
};

void DigitalTree::Destroy(Node *node)
{
  if (node->isleaf) delete (LeafNode*)node;
  else {
    for (vector<Node*>::iterator i = ((InnerNode*)node)->Children.begin();
	 i!=((InnerNode*)node)->Children.end();
	 ++i)
      if (*i) Destroy(*i);
    delete (InnerNode*)node;
  }
}

void DigitalTree::DestructiveStore(Node *node, SimpleVectorSet &S)
{
  if (node->isleaf) {
    S.insert(((Vector&)((LeafNode*)node)->leaf));
    delete (LeafNode*)node;
  }
  else {
    for (vector<Node*>::iterator i = ((InnerNode*)node)->Children.begin();
	 i!=((InnerNode*)node)->Children.end();
	 ++i)
      if (*i) DestructiveStore(*i, S);
    delete (InnerNode*)node;
  }
}

bool DigitalTree::DoSearch(Leaf min, Leaf max, 
			   Report report, int level, Node *node)
{
  if (node->isleaf) {
    Vector::pointer vi, mini, maxi;
    int pos = level;
    int count = level+1;
    for (vi = &(((Vector&)(((LeafNode*)node)->leaf))[pos]),
	   mini = &(((Vector&)(min))[pos]),
	   maxi = &(((Vector&)(max))[pos]);
	 count; vi--, mini--, maxi--, count--)
      if ((*vi)<(*mini) || (*vi)>(*maxi)) return true;
    return report(((LeafNode*)node)->leaf);
  }
  else {
    vector<Node*>::pointer ci;
    int count = ((Vector&)max)[level] - ((Vector&)min)[level] + 1;
    for (ci = &(((InnerNode*)node)->Children[Dimension + ((Vector&)min)[level]]); 
	 count; ci++, count--)
      if (*ci && !DoSearch(min, max, report, level-1, *ci)) 
	return false;
    return true;
  }
}

bool DigitalTree::Insert(Leaf leaf, bool CheckDup)
{
  InnerNode *n = root;
  int level;
  for (level = Dimension-1; level>=0; level--) {
    int pos = Dimension + ((Vector&)leaf)[level];
    if (n->Children[pos]) {
      if (n->Children[pos]->isleaf) {
	LeafNode *ol = (LeafNode*) n->Children[pos];
	if (CheckDup) {
	  // Check if equal
	  int i;
	  for (i = level-1; 
	       i>=0 && ((Vector&)(ol->leaf))[i] == ((Vector&)leaf)[i];
	       i--);
	  if (i<0) {
#if (TALKATIVE >= 3)
	    cerr << "$";
#endif
	    return false;
	  }
	}
	// Build a chain of inners up to tie 
	// FIXME: What about a compressed structure...?
	do {
	  InnerNode *i = new InnerNode(2 * Dimension + 1);
	  n->Children[pos] = i;
	  n = i, level--, pos = Dimension + ((Vector&)leaf)[level];
	} while (level > 0 
		 && (((Vector&)(ol->leaf))[level] 
		     == ((Vector&)leaf)[level]));
	// Put the old and new leaves at the end
	LeafNode *l = new LeafNode;
	l->leaf = leaf;
	n->Children[Dimension + ((Vector&)leaf)[level]] = l;
	n->Children[Dimension + ((Vector&)(ol->leaf))[level]] = ol;
	// we are done
	return true;
      }
      else { // Is inner node
	n = (InnerNode*) n->Children[pos];
	// go on
      }
    }
    else { // Is empty slot
      LeafNode *l = new LeafNode;
      l->leaf = leaf;
      n->Children[pos] = l;
      // we are done
      return true; 
    }
  }
}

class VectorSet {
  DigitalTree *tree;
  VectorSet(const VectorSet &);
  operator=(const VectorSet &);
  
public:
  VectorSet(int level) : tree(new DigitalTree(level+1)) {}
  ~VectorSet() { delete tree; }
  bool insert(Vector v, bool CheckDup = false) { 
    Vector *vv = new Vector(v);
    VectorRepository.push_back(vv);
    return tree->Insert(Leaf(vv), CheckDup); 
  }
  bool OrthogonalRangeSearch(Leaf min, Leaf max, 
			     Report report)
    { return tree->OrthogonalRangeSearch(min, max, report); }
  operator SimpleVectorSet();
  void DestructiveCopy(SimpleVectorSet &S)
    { tree->DestructiveCopy(S); }
};

static SimpleVectorSet *inserthackset;

static bool inserthack(const Leaf &y)
{
  inserthackset->insert((Vector&)y);
  return true;
}

VectorSet::operator SimpleVectorSet()
  return Set;
{
  inserthackset = &Set;
  Vector min(tree->Dimension), max(tree->Dimension);
  for (int i = 1; i<=tree->Dimension; i++)
    min(i) = -tree->Dimension, max(i) = tree->Dimension;
  OrthogonalRangeSearch(Leaf(&min), Leaf(&max), inserthack);
}

ostream &operator<<(ostream &s, const Vector &z)
{
  for (int i = 0; i<z.size(); i++)
    s << setw(4) << (int)(z[i]);
  return s;
}

void ClearVectorRepository()
{
  for (vector<Vector *>::iterator i = VectorRepository.begin();
       i!=VectorRepository.end();
       ++i)
    delete *i;
  VectorRepository = vector<Vector *>();
}

int HilbertDivide(Vector z, Vector y)
{
  // Find maximal integer f with f*y<=z in Hilbert-base sense.
  int maxfactor = INT_MAX;
  for (int i = 0; i<z.size(); i++) {
    if (y[i] > 0) {
      if (y[i] > z[i]) return 0;
      // here is z[i]>=y[i]>0.
      maxfactor = maxfactor <? (z[i] / y[i]);
    }
    else if (y[i] < 0) {
      if (y[i] < z[i]) return 0;
      // here is z[i]<=y[i]<0.
      maxfactor = maxfactor <? (z[i] / y[i]);
    }
  }
  return maxfactor;
}

void writeppi(ostream &c, Vector z, int n)
{
  bool first = true;
  for (int i = 0; i<n; i++) 
    if (z[i] > 0) {
      if (first) first = false;
      else c << " + ";
      for (int j = 1; j < z[i]; j++)
	c << i+1 << " + ";
      c << i+1;
    }
  c << "\t= ";
  first = true;
  for (int i = 0; i<n; i++) 
    if (z[i] < 0) {
      if (first) first = false;
      else c << " + ";
      for (int j = 1; j < -z[i]; j++)
	c << i+1 << " + ";
      c << i+1;
    }
  c << endl;
}

static int ppicount;

/* rangereport parameters */
static Vector rangemin, rangemax;
static int LastNonzeroPos;
#ifdef POSTCHECK
static Vector *RangeException;
#endif

static bool False(const Leaf &y)
{
#ifdef POSTCHECK
  if (&(Vector&)(y) == RangeException) return true;
#endif
  return false;
}

/////////////////

bool IsReducible(Vector &z, VectorSet &S)
{
  int i;
  rangemin = Vector(z.size()), rangemax = Vector(z.size());

  for (i = z.size(); i && !z(i); i--) rangemin(i) = rangemax(i) = 0;
  LastNonzeroPos = i;

  // Let x = z(LastNonzeroPos).

  // positive search is enough because there must be one summand that
  // is positive at LastNonzeroPos.
  for (i--; i; i--)
    if (z(i) >= 0) rangemin(i) = 0, rangemax(i) = z(i);
    else rangemin(i) = z(i), rangemax(i) = 0;

  rangemin(LastNonzeroPos) = 1, 
    rangemax(LastNonzeroPos) = z(LastNonzeroPos);
  return (!S.OrthogonalRangeSearch(Leaf(&rangemin), Leaf(&rangemax),
				   &False));
}

//
// Specialized code for generating all primitive partition identities
//

void reportx(Vector z)
{
  ppicount++;
#if (TALKATIVE>=1)
  cout << z << "\t"; writeppi(cout, z, z.size()); 
#endif
}

inline void RaisePPI(const Vector &v, int j, int k, int n, 
		     VectorSet &P, SimpleVectorSet &Pnew)
{
  Vector w = v;
  w(n+1)++, w(j)--, w(k)--;
  if (w(j) >= 0 && w(k) >= 0) { // w is irreducible 
    if (P.insert(w, true)) Pnew.insert(w), reportx(w);
  }
  else { // may be reducible, must try to reduce 
    int i;
    rangemin = Vector(n+1), rangemax = Vector(n+1);
    rangemin(n+1) = rangemax(n+1) = 0;
    // use full range at the remaining positions
    for (i = n; i; i--) {
      if (w(i) >= 0) rangemin(i) = 0, rangemax(i) = w(i);
      else rangemin(i) = w(i), rangemax(i) = 0;
    }
    // FIXME: maybe special-case k or j = n (only one sign is possible
    // in this case.)
    
    // at the position that increased (abs value), demand this value
    if (w(k) < 0) rangemin(k) = w(k), rangemax(k) = w(k);
    else rangemin(j) = w(j), rangemax(j) = w(j);
    // search!
    if (P.OrthogonalRangeSearch(Leaf(&rangemin), Leaf(&rangemax),
				&False)) {
      // try negative search
      for (i = n; i; i--) {
	if (w(i) <= 0) rangemin(i) = 0, rangemax(i) = -w(i);
	else rangemin(i) = -w(i), rangemax(i) = 0;
      }
      if (w(k) < 0) rangemin(k) = -w(k), rangemax(k) = -w(k);
      else rangemin(j) = -w(j), rangemax(j) = -w(j);
      if (P.OrthogonalRangeSearch(Leaf(&rangemin), Leaf(&rangemax),
				  &False)) {
	// didn't find reducer, but vector may already be known
	if (P.insert(w, true)) Pnew.insert(w), reportx(w);
      }
    }
  }
}

void ExtendPPI(SimpleVectorSet &Pn, int n)
{
  VectorSet P(n);
  SimpleVectorSet *Pold = new SimpleVectorSet;
  SimpleVectorSet *Pnew = new SimpleVectorSet;

  // Implementation of Algorithms 4.3.8, 4.3.11 from [Urbaniak] 

  // (1) Create the range-searchable set P of (n+1)-vectors from the
  // set Pn of n-vectors.
  cerr << "# Vectors copied from n = " << n << ": " << endl;
  for (SimpleVectorSet::iterator i = Pn.begin(); i!=Pn.end(); ++i) {
    Vector v(n+1);
    for (int j = 1; j <= n; j++) v(j) = (*i)(j);
    v(n+1) = 0;
    P.insert(v); Pold->insert(v); reportx(v);
  }
  // destroy Pn to save memory
  Pn = SimpleVectorSet();

  // (2) Add `(n+1) = a + b' identities.
  cerr << "# Vectors of type " << n+1 << " = a + b:" << endl;
  {
    Vector v(n+1);
    for (int j = 1; j <= n; j++) v(j) = 0;
    v(n+1) = +1;
    for (int p = 1; p<=(n+1)/2; p++) {
      // takes care of case: p = n+1-p.
      v(p)--, v(n+1-p)--;
      Pnew->insert(v); 
      P.insert(v); reportx(v);
      v(p)++, v(n+1-p)++;
    }
  }

  // (3) Build all other primitive identities with exactly one
  // component of (n+1).
  cerr << "# Vectors of P" << 1 << "(" << n+1 << "):" << endl;
  for (SimpleVectorSet::iterator i = Pold->begin(); i!=Pold->end(); ++i) {
    Vector v = *i;
#if (TALKATIVE>=2)
    cerr << "Raising " << v << endl;
#endif
    for (int j = 1; j<=(n+1)/2; j++) {
      int k = (n+1) - j;
      if (v(j) > 0 || v(k) > 0) { // otherwise, w reducible by v
	RaisePPI(v, j, k, n, P, *Pnew);
      }
      if (v(j) < 0 || v(k) < 0) { // otherwise, w reducible by v
	RaisePPI(-v, j, k, n, P, *Pnew);
      }
    }
  }
  //
  delete Pold;
  Pold = Pnew;
  Pnew = new SimpleVectorSet;

  // (4) Build all other primitive identities with exactly (t+1)
  // components of (n+1).
  for (int t = 1; t<n; t++) {
    cerr << "# Vectors of P" << t+1 << "(" << n+1 << "):" << endl;
    for (SimpleVectorSet::iterator i = Pold->begin(); i!=Pold->end(); ++i) {
      Vector v = *i;
#if (TALKATIVE>=2)
      cerr << "Raising " << v << endl;
#endif
      for (int j = 1; j<=(n+1)/2; j++) {
	int k = (n+1) - j;
	if (v(j) > 0 || v(k) > 0) { // otherwise, w reducible by v
	  RaisePPI(v, j, k, n, P, *Pnew);
	}
      }
    }
    delete Pold;
    Pold = Pnew;
    Pnew = new SimpleVectorSet;
  }

#ifdef POSTCHECK
  cerr << "# Postcheck..." << endl;
  SimpleVectorSet S;
  VectorSet::iterator i;
  for (i = P.begin(); i!=P.end(); ++i) {
    RangeException = &((Vector&)(*i));
    Vector v = *i;
    if (IsReducible(v, P)) {
      cerr << "Haha... a reducible vector: " << *i << endl;
      ppicount--;
    }
    else {
      if (!S.insert(*i).second) {
	cerr << "Hoho... a duplicate: " << *i << endl;
      }
    }
  }
  RangeException = 0;
  return S;
#else
  P.DestructiveCopy(Pn);
#endif

}

int main(int argc, char *argv[])
{
  // PPI n = 5.
  int n = 0;
  if (argc >= 2) {
    sscanf(argv[1], "%d", &n);
  }
  if (!n) n = 5;

  // Setup PPI set for n=2
  SimpleVectorSet V;
  Vector v(2); v(1) = -2, v(2) = +1; V.insert(v);
  for (int i = 2; i<n; i++) {
    ppicount = 0;
    cerr << "### Extending to n = " << i+1 << endl;
    ExtendPPI(V, i);
    ClearVectorRepository();
    cerr << "### This makes " << ppicount 
	 << " PPI up to sign, " << V.size() << endl;
  }

}

/* $Log$
 * Revision 1.24  1999/03/09 17:36:01  mkoeppe
 * Clean up.
 *
 * Revision 1.23  1999/03/09 16:46:07  mkoeppe
 * Too cool to be true. Up to n=17 everything is computed in `zero
 * time'. From n=18 the memory load is too high.
 *
 * Revision 1.22  1999/03/09 15:12:56  mkoeppe
 * *** empty log message ***
 *
 * Revision 1.21  1999/03/09 00:53:49  mkoeppe
 * Using a simple `digital tree' instead. n=11 takes 3sec.
 *
 * Revision 1.20  1999/03/08 20:58:23  mkoeppe
 * Ok, 11 takes 53sec.
 *
 * Revision 1.19  1999/03/08 16:59:16  mkoeppe
 * Some optimization.
 *
 * Revision 1.18  1999/03/07 18:59:41  mkoeppe
 * Added some special-purpose reduction code, for use in iterative
 * computation (4.3.11) and post-check for reducibility.
 *
 * Revision 1.17  1999/03/07 16:02:19  mkoeppe
 * n=14 still need POST_CHECK, but everything else quite ok now.
 *
 * Revision 1.16  1999/03/06 16:59:20  mkoeppe
 * Added reducibility checks.
 *
 * Revision 1.15  1999/03/05 17:39:27  mkoeppe
 * Remove obsolete check. Gives back memory.
 *
 * Revision 1.14  1999/03/05 17:09:51  mkoeppe
 * Now the last nonzero component is always positive. Took some more care
 * in the positive/negative reduction code. Hopefully this will fix n=14.
 *
 * Revision 1.13  1999/03/05 14:36:17  mkoeppe
 * Minor changes.
 *
 * Revision 1.12  1999/03/05 12:33:27  mkoeppe
 * Unified the search modes; they perform about equally.
 *
 * Revision 1.11  1999/03/04 23:53:02  mkoeppe
 * Initial implementation of R. Urbaniak's PPI algorithms.
 * */