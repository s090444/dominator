/*
 * Compound.cpp
 *
 *  Created on: Jun 1, 2011
 *      Author: markus
 */

#include <simulation/Compound.hpp>

namespace sim {

__Compound::__Compound()
	: __Object(COMPOUND), m_nextID(0)
{
	m_matrix = Mat4f::identity();
}

__Compound::__Compound(const Mat4f& matrix)
	: __Object(COMPOUND), m_nextID(0)
{
	m_matrix = matrix;
}

__Compound::__Compound(const Vec3f& position)
	: __Object(COMPOUND), m_nextID(0)
{
	m_matrix = Mat4f::translate(position);
}

__Compound::~__Compound()
{
	m_nodes.clear();
	m_joints.clear();
}

float __Compound::convexCastPlacement(bool apply)
{
	float maximum = -1000.0f;
	for (std::list<Object>::iterator itr = m_nodes.begin(); itr != m_nodes.end(); ++itr) {
		float current = (*itr)->convexCastPlacement(false);
		if (current > maximum) maximum = current;
	}
	Mat4f matrix = m_matrix;
	matrix._42 = maximum + 0.0001f;
	if (apply)
		setMatrix(matrix);
	return matrix._42;
}

void __Compound::save(__Compound& compound, rapidxml::xml_node<>* node, rapidxml::xml_document<>* doc)
{
	// foreach child_node
	for (std::list<Object>::iterator itr = compound.m_nodes.begin();
				itr != compound.m_nodes.end(); ++itr) {
					__Object::save(*itr->get(), node, doc);
	}

	// foreach joint
	for (std::list<Joint>::iterator itr = compound.m_joints.begin();
				itr != compound.m_joints.end(); ++itr) {
					__Joint::save(*itr->get(), node, doc);
	}
}

Compound __Compound::load(rapidxml::xml_node<>* nodes)
{
	using namespace rapidxml;

	Compound result = Compound(new __Compound());
	// load m_matrix

	// type attribute
	xml_attribute<>* attr = nodes->first_attribute();

	// matrix attribute
	attr = attr->next_attribute();
	result->m_matrix.assign(attr->value());
	
	
	for (xml_node<>* node = nodes->first_node(); node; node = node->next_sibling()) {
		std::string type = node->name();
		if(type == "object") {
			Object obj = __Object::load(node);
	 		result->add(obj);
		}
		if(type == "joint") {
			//std::list<Object>& list = std::list<Object>(result->m_nodes);
			Joint joint = __Joint::load(result->m_nodes, node);
	 		result->m_joints.push_back(joint);
		}
	}
	// foreach element "object" in node
	// 		Object obj = __Object::load(element)
	// 		result->add(obj)

	// foreach element "joint" in node
	// 		Joint joint = __Joint::load(element)
	// 		result->m_joints.push_back(joint)

	return result;
}

Hinge __Compound::createHinge(const Vec3f& pivot, const Vec3f& pinDir, const Object& child, const Object& parent)
{
	if (child && child != parent) {
		Vec3f pin = pinDir % m_matrix;
		Hinge hinge = __Hinge::create(pivot, pin, child, parent);
		m_joints.push_back(hinge);
		return hinge;
	}
	Hinge result;
	return result;
}

BallAndSocket __Compound::createBallAndSocket(const Vec3f& pivot, const Vec3f& pinDir, const Object& child, const Object& parent)
{
	if (child && child != parent) {
		Vec3f pin = pinDir % m_matrix;
		BallAndSocket ball = __BallAndSocket::create(pivot, pin, child, parent);
		m_joints.push_back(ball);
		return ball;
	}
	BallAndSocket result;
	return result;
}

void __Compound::add(Object object)
{
	object->setID(m_nextID++);
	Mat4f matrix = object->getMatrix() * m_matrix;
	object->setMatrix(matrix);
	m_nodes.push_back(object);
}

void __Compound::setMatrix(const Mat4f& matrix)
{
	Mat4f inverse = m_matrix.inverse();
	for (std::list<Object>::iterator itr = m_nodes.begin();
				itr != m_nodes.end(); ++itr) {
		Mat4f newMatrix = (*itr)->getMatrix();
		newMatrix *= inverse * matrix;
		(*itr)->setMatrix(newMatrix);
	}
	m_matrix = matrix;
}

void __Compound::setFreezeState(int state)
{
	for (std::list<Object>::iterator itr = m_nodes.begin();
				itr != m_nodes.end(); ++itr) {
		(*itr)->setFreezeState(state);
	}
}


int __Compound::getFreezeState()
{
	int state = (*m_nodes.begin())->getFreezeState();
	for (std::list<Object>::iterator itr = m_nodes.begin();
				itr != m_nodes.end(); ++itr) {
		if ((*itr)->getFreezeState() != state)
			return 0;
	}
	return state;
}

bool __Compound::contains(const NewtonBody* const body)
{
	for (std::list<Object>::iterator itr = m_nodes.begin();
			itr != m_nodes.end(); ++itr) {
		if ((*itr)->contains(body))
			return true;
	}
	return false;
}

bool __Compound::contains(const Object& object)
{
	if (object.get() == this)
		return true;
	for (std::list<Object>::iterator itr = m_nodes.begin();
			itr != m_nodes.end(); ++itr) {
		if ((*itr)->contains(object))
			return true;
	}
	return false;
}

void __Compound::genBuffers(ogl::VertexBuffer& vbo)
{
	for (std::list<Object>::iterator itr = m_nodes.begin();
			itr != m_nodes.end(); ++itr) {
		(*itr)->genBuffers(vbo);
	}
}

void __Compound::render()
{
	for (std::list<Object>::iterator itr = m_nodes.begin();
			itr != m_nodes.end(); ++itr) {
		(*itr)->render();
	}
}

}