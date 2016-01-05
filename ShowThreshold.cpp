#include <fantom/algorithm.hpp>
#include <fantom/register.hpp>
#include <fantom/graphics.hpp>
#include <fantom/fields.hpp>

using namespace fantom;

namespace {

	class ShowThreshold : public VisAlgorithm {

		std::unique_ptr< Primitive > m_spheres;

	public:

		struct Options : public VisAlgorithm::Options {
			Options( fantom::Options::Control& control ) :
				VisAlgorithm::Options( control )
			{
				add< TensorFieldDiscrete< Tensor< double, 1 > > >( "TensorField", "Scalar tensorfield" );
				add< float >( "Threshold", "Target visualization threshold", 8e-4 ); 
			}
		};

		struct VisOutputs : public VisAlgorithm::VisOutputs {
			VisOutputs( fantom::VisOutputs::Control& control ) :
				VisAlgorithm::VisOutputs( control )
			{
				addGraphics( "thresholdGlyphs" );
			}
		};

		ShowThreshold( InitData& data ) :
			VisAlgorithm( data )
		{

		}	

		virtual void execute( const Algorithm::Options& options, const volatile bool& abortFlag ) override {
			auto tensorField = options.get< TensorFieldDiscrete< Tensor<  double, 1 > > >( "TensorField" );
			float thresh = options.get< float >( "Threshold" );
			m_spheres = getGraphics( "thresholdGlyphs" ).makePrimitive();

			auto evaluator = tensorField->makeDiscreteEvaluator();
			debugLog() << "EvalValues: " << evaluator->numValues() << std::endl;
			
			std::shared_ptr< const Grid< 3 > > grid = std::dynamic_pointer_cast< const Grid< 3 > >( tensorField->domain() );
			const ValueArray< Point3 >& points = grid->points();

			for (int i = 0; i < evaluator->numValues(); i++)
			{
				Tensor< double, 1 > tensor = evaluator->value( i );
				if( tensor[0] > thresh ) m_spheres->addSphere( points[i], 0.25, Color( 1.0, 0.0, 0.0, 1.0 ) );
			}
		}

	};

	AlgorithmRegister< ShowThreshold > reg( "VisPraktikum/ShowThreshold", "Visualizes a certain threshold of a scalar dataset" );

}