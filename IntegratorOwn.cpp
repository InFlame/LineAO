#include <fantom/algorithm.hpp>
#include <fantom/register.hpp>
#include <fantom/graphics.hpp>
#include <fantom/fields.hpp>

using namespace fantom;

namespace {

	class Integrator : public VisAlgorithm {

		std::unique_ptr< Primitive > m_streamLines;

		std::unique_ptr< Primitive > m_selectionPoint1;
		std::unique_ptr< Primitive > m_selectionPoint2;
		std::unique_ptr< Primitive > m_selectionLine;
		std::unique_ptr< Primitive > m_startingPoints;

		std::unique_ptr< Manipulator > m_manipulator1;
		std::unique_ptr< Manipulator > m_manipulator2;
		std::unique_ptr< Manipulator > m_manipulator3;

		Point3 p1;
		Point3 p2;

		bool euler;

	public:
		struct Options : public VisAlgorithm::Options {
			Options( fantom::Options::Control& control ) :
				VisAlgorithm::Options( control )
			{
				add< TensorFieldInterpolated< 3, Vector3 > >( "Field", "3D vector field" );
				add< InputChoices >( "Algorithm", "Choose an integration algorithm", std::vector< std::string >{ "Euler", "Runge-Kutta" }, "Euler" );
				add< int >( "Starting points", "Number of starting points", 20 );
				add< float >( "Step size", "Integration step size", 0.1 );
			}
		};

		struct VisOutputs : public VisAlgorithm::VisOutputs {
			VisOutputs( fantom::VisOutputs::Control& control ) :
				VisAlgorithm::VisOutputs( control )
			{
				addGraphics( "streamlines" );
				addGraphics( "selectionLine" );
				addGraphics( "selectionPoint1" );
				addGraphics( "selectionPoint2" );
				addGraphics( "startingPoints" );
			}
		};

		Integrator( InitData& data ) :
			VisAlgorithm( data )
		{
			m_manipulator1 = getGraphics( "selectionPoint1" ).makeManipulator();
			m_manipulator2 = getGraphics( "selectionPoint2" ).makeManipulator();
			//m_manipulator3 = getGraphics( "selectionLine" ).makeManipulator();

			m_manipulator1->setDragCallback( std::bind( 
				&Integrator::move, 
				this,
				1,
				std::placeholders::_1,
				std::placeholders::_2,
				std::placeholders::_3,
				std::placeholders::_4 
			) );

			m_manipulator2->setDragCallback( std::bind( 
				&Integrator::move, 
				this,
				2,
				std::placeholders::_1,
				std::placeholders::_2,
				std::placeholders::_3,
				std::placeholders::_4 
			) );


			m_manipulator1->primitive().addSphere( Point3( 0.0, 0.0, -1.0 ), 0.05, Color( 1.0, 0.0, 0.0, 0.5 ) );
			m_manipulator2->primitive().addSphere( Point3( 0.0, 0.0, 1.0 ), 0.05, Color( 1.0, 0.0, 0.0, 0.5 ) );
		}

		virtual void execute( const Algorithm::Options& options, const volatile bool& /*abortFlag*/ ) override {
			m_startingPoints = getGraphics( "startingPoints").makePrimitive();
			m_streamLines = getGraphics( "streamlines" ).makePrimitive();

			if( options.get< std::string >( "Algorithm" ) == "Euler" ) euler = true;
			else euler = false;

			auto field = options.get< TensorFieldInterpolated< 3, Vector3 > >( "Field" );

			// check if spheres lie within data boundingbox
			std::shared_ptr< const Grid< 3 > > grid = std::dynamic_pointer_cast< const Grid< 3 > >( field->domain() );

			if( grid->index( grid->locate( p1 ) ) == 0 || grid->index( grid->locate( p2 ) ) == 0 ) {
				infoLog() << "Starting points out of bounds!" << std::endl;
				return;
			}

			Vector3 startingLine = p2 - p1;
			float length = norm( startingLine );
			//m_startingPoints->addArrow( Point3( 0.0, 0.0, 0.0 ) p1, startingLine, 0.1, Color( 0.0, 1.0, 0.0 ) );
			//m_startingPoints->addSphere( p1 - Point3(0.0, 0.0, 1.0), 0.1, Color( 0.0, 1.0, 0.0 ) );
			//m_startingPoints->addSphere( p2 + Point3(0.0, 0.0, 1.0), 0.1, Color( 0.0, 1.0, 0.0 ) );

			std::vector< std::vector< Point3 > > vertices;

			std::vector< Point3 > startingPoints( options.get< int >( "Starting points" ) );
			
			float distance = length / (float)startingPoints.size();
			for (int i = 0; i < startingPoints.size(); i++)
			{
				startingPoints[i] = normalized( startingLine ) * ( distance * i ) + p1;
				vertices.push_back( std::vector< Point3 >() );
				//m_startingPoints->addSphere( startingPoints[i], 0.05, Color( 0.0, 1.0, 0.0, 1.0 ) );
			}

			float stepSize = options.get< float >( "Step size" );
			const ValueArray< Point3 >& points = grid->points();

			
			if( euler ) {
				// ------------- Euler -------------
				#pragma omp parallel for 
				for( int i=0; i<startingPoints.size(); i++ ) {
					auto evaluator = field->makeEvaluator();
					while( grid->index( grid->locate( startingPoints[i] ) ) != 0 ) {
						vertices[i].push_back( startingPoints[i] );

						// get next interpolated position
						Cell cell = grid->locate( startingPoints[i] );
						if( evaluator->reset( startingPoints[i] ) ) {
							Tensor< double, 3 > vector = evaluator->value();
							startingPoints[i] = startingPoints[i] + ( normalized( vector ) * stepSize );
						} else break;

						vertices[i].push_back( startingPoints[i] );
					}
				}
			} else {

				// ------------ runge-kutta -------------
				#pragma omp parallel for
				for( int i=0; i<startingPoints.size(); i++ ) {
					auto evaluator = field->makeEvaluator();
					while( grid->index( grid->locate( startingPoints[i] ) ) != 0 ) {
						vertices[i].push_back( startingPoints[i] );

						Cell cell = grid->locate( startingPoints[i] );

						Vector3 q1, q2, q3, q4;

						//	q1
						if( !evaluator->reset( startingPoints[i] ) ) break;
						q1 = evaluator->value();

						// q2
						if( !evaluator->reset( startingPoints[i] + ( stepSize / 2 ) * q1 ) ) break;
						q2 = evaluator->value();

						// q3
						if( !evaluator->reset( startingPoints[i] + ( stepSize / 2 ) * q2 ) ) break;;
						q3 = evaluator->value();

						// q4
						if( !evaluator->reset( startingPoints[i] + stepSize * q3 ) ) break;
						q4 = evaluator->value();

						// n+1			  = ...
						startingPoints[i] = startingPoints[i] + ( stepSize / 6 ) * ( q1 + 2*q2 + 2*q3 + q4 );

						vertices[i].push_back( startingPoints[i] );
					}
				}
			}

			for( int i=0; i<vertices.size(); i++ ) {
				m_streamLines->add( Primitive::LINES ).setColor( Color( 1.0, 0.0, 0.0 ) ).setVertices( vertices[i] );
			}

		}

		Vector3 center( int i ) const {
			/*Vector4 x;
			if( i == 1 ) Vector4 x = Vector4( 0.0, 0.0, 0.0, 1.0 ) * m_manipulator1->view().matrix();
			else Vector4 x = Vector4( 0.0, 0.0, 0.0, 1.0 ) * m_manipulator2->view().matrix();*/
			Vector4 x = Vector4( 0.0, 0.0, 0.0, 1.0 ) * m_manipulator1->view().matrix();
			return Vector3( x[0] / x[3], x[1] / x[3], x[2] / x[3] );
		}

		void move( int i, Point3 s0, Vector3 s, Point3 t0, Vector3 t ) {
			double d = ( t0 - s0 ) * ( t0 - s0 );
            if( d == 0.0 )
                return;
            Vector3 n = s - ( t0 - s0 ) * ( s * ( t0 - s0 ) ) / d;
            double m = ( n * ( center( i ) - s0 ) ) / ( n * s );
            if( i == 1 ) {
            	m_manipulator1->view().translate( ( t0 - s0 ) + ( t - s ) * m );
            	p1 = Point3( 
            		m_manipulator1->view().matrix()[3 * 4 + 0],
            		m_manipulator1->view().matrix()[3 * 4 + 1],
            		m_manipulator1->view().matrix()[3 * 4 + 2] - 1.0
            	);
            	//debugLog() << m_manipulator1->view().matrix() << std::endl;
            }
            else {
            	m_manipulator2->view().translate( ( t0 - s0 ) + ( t - s ) * m );
            	p2 = Point3( 
            		m_manipulator2->view().matrix()[3 * 4 + 0],
            		m_manipulator2->view().matrix()[3 * 4 + 1],
            		m_manipulator2->view().matrix()[3 * 4 + 2] + 1.0
            	);
            	//debugLog() << m_manipulator2->view().matrix() << std::endl;
            }
            
		}

	};

	AlgorithmRegister< Integrator > reg( "VisPraktikum/IntegratorOwn", "Line integration" );
}