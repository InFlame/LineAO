#include <fantom/algorithm.hpp>
#include <fantom/register.hpp>
#include <fantom/graphics.hpp>
#include <fantom/fields.hpp>

#include "Integrator.cpp"

using namespace fantom;

namespace {

	class RungeKutta : public Integrator {

	public:

		RungeKutta( InitData& data ) :
			Integrator( data )
		{

		}

		void execute( const Algorithm::Options& options, const volatile bool& abortFlag ) override {
			std::shared_ptr< const LineSet > set = options.get< const LineSet >( "Seed line" );
			Integrator::execute( options, abortFlag );

			// check seedline vs grid bounding box
			m_numPoints = m_seedLine->getNumPoints();
			if( m_grid->index( m_grid->locate( m_seedLine->getPointOnLine( 0, 0 ) ) ) == 0 ||
				m_grid->index( m_grid->locate( m_seedLine->getPointOnLine( 0, m_numPoints-1 ) ) ) == 0 )
			{
				infoLog() << "Seed points out of bounds" << std::endl;
				return;
			}

			std::vector< Point3 > startingPoints( m_numPoints );
			for( int i=0; i<m_numPoints; i++ ) {
				startingPoints[i] = m_seedLine->getPoint( i );
				m_vertices.push_back( std::vector< Point3 >() );
			}

			#pragma omp parallel for
			for( int i=0; i<startingPoints.size(); i++ ) {
				auto evaluator = m_field->makeEvaluator();
				while( m_grid->index( m_grid->locate( startingPoints[i] ) ) != 0 ) {
					m_vertices[i].push_back( startingPoints[i] );

					Vector3 q1, q2, q3, q4;

					//	q1
					if( !evaluator->reset( startingPoints[i] ) ) break;
					q1 = evaluator->value();

					// q2
					if( !evaluator->reset( startingPoints[i] + ( m_stepSize / 2 ) * q1 ) ) break;
					q2 = evaluator->value();

					// q3
					if( !evaluator->reset( startingPoints[i] + ( m_stepSize / 2 ) * q2 ) ) break;;
					q3 = evaluator->value();

					// q4
					if( !evaluator->reset( startingPoints[i] + m_stepSize * q3 ) ) break;
					q4 = evaluator->value();

					// n+1			  = ...
					startingPoints[i] = startingPoints[i] + ( m_stepSize / 6 ) * ( q1 + 2*q2 + 2*q3 + q4 );

					m_vertices[i].push_back( startingPoints[i] );
				}
			}

			Integrator::makeLineSet( options );
		}

	};

	AlgorithmRegister< RungeKutta > reg( "VisPraktikum/RungeKutta", "RungeKutta integration" );

}