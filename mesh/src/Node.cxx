/*
 * Node.cxx
 *
 *  Created on: 23 janv. 2012
 *      Authors: CDMAT
 */

#include "Node.hxx"


//----------------------------------------------------------------------
Node::Node( void )
//----------------------------------------------------------------------
{
	_numberOfCells = 0 ;
	_numberOfFaces = 0 ;
	_groupNames=std::vector<std::string>(0);
	_region=-1;
}

//----------------------------------------------------------------------
Node::~Node( void )
//----------------------------------------------------------------------
{
}

//----------------------------------------------------------------------
Node::Node( const Node& node )
//----------------------------------------------------------------------
{
	_point = node.getPoint() ;
	_cellsId = node.getCellsId() ;
	_facesId = node.getFacesId() ;
	_numberOfCells = node.getNumberOfCells() ;
	_numberOfFaces = node.getNumberOfFaces() ;
	_groupNames=node.getGroupNames();
	_region=node.getRegion();
}

//----------------------------------------------------------------------
Node::Node( const int numberOfCells, const int numberOfFaces, const Point p )
//----------------------------------------------------------------------
{

	_point = p ;
	_numberOfCells = numberOfCells ;
	_numberOfFaces = numberOfFaces ;
	_cellsId = IntTab(_numberOfCells,0);
	_facesId = IntTab(_numberOfFaces,0);
	_groupNames=std::vector<std::string>(0);
	_region=-1;
}

//----------------------------------------------------------------------
IntTab
Node::getCellsId( void ) const 
//----------------------------------------------------------------------
{
	return _cellsId ;
}

//----------------------------------------------------------------------
IntTab
Node::getFacesId( void ) const 
//----------------------------------------------------------------------
{
	return _facesId ;
}

//----------------------------------------------------------------------
int
Node::getNumberOfCells( void ) const 
//----------------------------------------------------------------------
{
	return _numberOfCells ;
}

//----------------------------------------------------------------------
int
Node::getNumberOfFaces( void ) const 
//----------------------------------------------------------------------
{
	return _numberOfFaces ;
}

//----------------------------------------------------------------------
Point
Node::getPoint( void ) const
//----------------------------------------------------------------------
{
	return _point ;
}

//----------------------------------------------------------------------
double
Node::x( void ) const 
//----------------------------------------------------------------------
{
	return _point.x() ;
}

//----------------------------------------------------------------------
double
Node::y( void ) const 
//----------------------------------------------------------------------
{
	return _point.y() ;
}

//----------------------------------------------------------------------
double
Node::z( void ) const 
//----------------------------------------------------------------------
{
	return _point.z() ;
}

std::vector<std::string>
Node::getGroupNames(void) const
{
	return _groupNames;
}

std::string
Node::getGroupName(int igroup) const
{
	return _groupNames[igroup];
}

void
Node::setGroupName(const std::string groupName)
{
	_groupNames.push_back(groupName);
	_region=0;
}

bool
Node::isBorder(void)
{
	if (_region==0)
		return true;
	else
		return false;
}

int
Node::getRegion(void) const
{
	return _region;
}

//----------------------------------------------------------------------
void
Node::addFaceId (const int numFace, const int faceId )
//----------------------------------------------------------------------
{
	_facesId(numFace) = faceId ;
}

//----------------------------------------------------------------------
void
Node::addCellId (const int numCell, const int cellId )
//----------------------------------------------------------------------
{
	_cellsId(numCell) = cellId ;
}

//----------------------------------------------------------------------
double
Node::distance( const Node& n ) const
//----------------------------------------------------------------------
{
	double distance=_point.distance(n.getPoint());
	return distance ;
}

//----------------------------------------------------------------------
int
//----------------------------------------------------------------------
Node::getFaceId(int localId) const
//----------------------------------------------------------------------
{
	return _facesId[localId];
}

const Node& 
Node::operator= ( const Node& node )
//----------------------------------------------------------------------
{
   _point = node.getPoint()  ;
   _cellsId = node.getCellsId() ;
   _facesId = node.getFacesId() ;
   _numberOfCells = node.getNumberOfCells() ;
   _numberOfFaces = node.getNumberOfFaces() ;
   _groupNames = node.getGroupNames();	
	return *this;
}
