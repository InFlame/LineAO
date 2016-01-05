#include <fantom/algorithm.hpp>
#include <fantom/register.hpp>
#include <fantom/graphics.hpp>
#include <fantom/fields.hpp>

#include "Integrator.cpp"

using namespace fantom;

namespace {

	class Euler : public Integrator {

	public:

		Euler( InitData& data ) :
			Integrator( data )
		{

		}

		void execute( const Algorithm::Options& options, const volatile bool& abortFlag ) override {
			std::shared_ptr< const LineSet > set = options.get< const LineSet >( "Seed line" );
			Integrator::execute( options, abortFlag );

			float epsilon = 0.00001;

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
			for( int i=0; i<m_numPoints; i++ ) {
				auto evaluator = m_field->makeEvaluator();
				while( m_grid->index( m_grid->locate( startingPoints[i] ) ) != 0 ) {
					m_vertices[i].push_back( startingPoints[i] );

					if( evaluator->reset( startingPoints[i] ) ) {
						Tensor< double, 3 > vector = evaluator->value();

						// adaptive step size
						Point3 tempStart = startingPoints[i];
						Point3 step = startingPoints[i] + ( normalized( vector ) * m_stepSize );
						Point3 halfStep = tempStart + ( normalized( vector ) * ( m_stepSize / 2 ) );
						halfStep = halfStep + ( normalized( vector ) * ( m_stepSize / 2 ) );

						if( startingPoints[i] == halfStep ) {
							startingPoints[i] = step;
							m_stepSize *= 2;
						} else if( norm( step - halfStep ) < epsilon ) startingPoints[i] = halfStep;
						else if( norm( step - halfStep ) > epsilon ) m_stepSize /= 2;
					} else break;
				}
			}

			Integrator::makeLineSet( options );
		}

	};

	AlgorithmRegister< Euler > reg( "VisPraktikum/Euler", "Euler integration" );

}
