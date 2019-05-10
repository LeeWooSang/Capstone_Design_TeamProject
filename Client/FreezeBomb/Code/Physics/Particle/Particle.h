#pragma once
#include "../Physics.h"

namespace Physics
{
	//A particle is the simplest object that can be simulated in the
    //physics system.

	/*It has position data (no orientation data), along with
     * velocity. It can be integrated forward through time, and have
     * linear forces, and impulses applied to it. The particle manages
     * its state and allows access through a set of methods.
	 */
	class Particle
	{
	protected:
		 /**
         * @name Characteristic Data and State
         *
         * This data holds the state of the particle. There are two
         * sets of data: characteristics and state.
         *
         * Characteristics are properties of the particle
         * independent of its current kinematic situation. This
         * includes mass, moment of inertia and damping
         * properties. Two identical particles will have the same
         * values for their characteristics.
         *
         * State includes all the characteristics and also includes
         * the kinematic situation of the particle in the current
         * simulation. By setting the whole state data, a particle's
         * exact game state can be replicated. Note that state does
         * not include any forces applied to the body. Two identical
         * rigid bodies in the same simulation will not share the same
         * state values.
         *
         * The state values make up the smallest set of independent
         * data for the particle. Other state data is calculated
         * from their current values. When state data is changed the
         * dependent values need to be updated: this can be achieved
         * either by integrating the simulation, or by calling the
         * calculateInternals function. This two stage process is used
         * because recalculating internals can be a costly process:
         * all state changes should be carried out at the same time,
         * allowing for a single call.
         *
         * @see calculateInternals
         */
        /*@{*/

		//������ ������ �� (inverse)�� �����մϴ�.
		//�� ������ �����ϴ� ���� �� �����մϴ�. 
		//�ֳ��ϸ� ������ �� �����ϰ� �ǽð����� �ùķ��̼��� ����ϸ� 
		//���� ���� (������ ������)�� ������ ���� ��ü (��ġ �ùķ��̼ǿ����� ������ �Ҿ����� ��ü)�� ���� ���� �� �����մϴ�.
		real inverseMass;


		 /**
         * Holds the amount of damping applied to linear
         * motion. Damping is required to remove energy added
         * through numerical instability in the integrator.
         */


		/*
		���� ��� ����Ǵ� ���� ���� �����մϴ�. 
		����� ���б��� ��ġ �� �Ҿ������� ���� �߰� �� �������� �����ϴ� �� �ʿ��մϴ�.
		*/
        real damping;

		 /**
         * Holds the linear position of the particle in
         * world space.
         */
        PVector3 position;

		 /**
         * Holds the linear velocity of the particle in
         * world space.
         */
        PVector3 velocity;


		        /*@}*/

        /**
         * @name Force Accumulators
         *
         * These data members store the current force and
         * global linear acceleration of the particle.

		 
		�� ������ ����� ���� ���� ������ �۷ι� ���� ������ �����մϴ�
         */

        /*@{*/

        /**
         * Holds the accumulated force to be applied at the next
         * simulation iteration only. This value is zeroed at each
         * integration step.
		 
			���� �� ���� ���� �ùķ��̼� �ݺ��ÿ��� �����մϴ�.
			�� ���� �� ���� �ܰ迡�� 0�Դϴ�
         */


        PVector3 forceAccum;

        /**
         * Holds the acceleration of the particle.  This value
         * can be used to set acceleration due to gravity (its primary
         * use), or any other constant acceleration.
		 
		������ ���ӵ��� �����մϴ�. 
		�� ���� �߷� (�� ���) �Ǵ� ��Ÿ ������ �������� ���� ������ �����ϴ� �� ����� �� �ֽ��ϴ�.
         */
        PVector3 acceleration;

        /*@}*/

		public:

	};

}