using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing;
using System.Data;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace Hyperion
{
	public partial class ZoomControl : UserControl
	{
		// Declare an event so the form using this control can hook into when the values change
		public delegate void ZoomControlChangedEvent( object sender, float value );
		public event ZoomControlChangedEvent OnValueChanged;

		// We also want to declare a public member, to allow for accessing the value at any time from outside of this class
		private float m_ZoomValue;
		private float m_MinZoom;
		private float m_MaxZoom;
		private float m_StepValue;
		private float m_ResetValue;

		public float ZoomValue
		{
			get { return m_ZoomValue; }
			private set
			{
				// Clamp the value by the bounds set
				m_ZoomValue = Math.Max( m_MinZoom, Math.Min( m_MaxZoom, value ) );
				UpdateUI();

				// Call event, so listeners can update what they need
				OnValueChanged( this, m_ZoomValue );
			}
		}

		public ZoomControl( float inMinValue = 0.1f, float inMaxValue = 10.0f, float inStartValue = 1.0f, float stepDelta = 0.1f )
		{
			// Throw an exception if the start value is out of bounds set, or the bounds are invalid
			if( inMinValue <= 0.0f || inMinValue > inMaxValue || inStartValue < inMinValue || inStartValue > inMaxValue )
			{
				throw new ArgumentOutOfRangeException( "The range set for zoom control is invalid, or the default value is outside of the bounds set" );
			}

			m_ZoomValue		= inStartValue;
			m_MinZoom		= inMinValue;
			m_MaxZoom		= inMaxValue;
			m_ResetValue	= inStartValue;

			// Check step delta, this is the value added or subtracted from the zoom value when buttons are pressed
			if( stepDelta <= 0 || stepDelta >= ( inMaxValue - inMinValue ) )
			{
				// Were going to default to 10% steps
				m_StepValue = 0.1f;
			}
			else
			{
				m_StepValue = stepDelta;
			}

			// Hook into enabled/disabled events
			EnabledChanged += ZoomControl_EnabledChanged;

			// Get the UI setup
			InitializeComponent();
			UpdateUI();
		}

		// Custom function to set a 'reset' value, so whenver the user double clicks the percentage label, or the reset event is manually
		// called, it will reset the zoom to this value, when constructed, it gets set to the starting value
		public bool SetResetValue( float inValue )
		{
			if( inValue < m_MinZoom || inValue > m_MaxZoom )
			{
				return false;
			}

			m_ResetValue = inValue;
			return true;
		}

		// Custom functions to manually zoom in/out that are public
		public void IncrementZoom()
		{
			IncrementZoom( m_StepValue );
		}

		public void DecrementZoom()
		{
			DecrementZoom( m_StepValue );
		}

		public void IncrementZoom( float inCustomStep )
		{
			if( m_ZoomValue < m_MaxZoom )
			{
				ZoomValue += inCustomStep;
			}
		}

		public void DecrementZoom( float inCustomStep )
		{
			if( m_ZoomValue > m_MinZoom )
			{
				ZoomValue -= inCustomStep;
			}
		}

		public void SetStepAmount( float inStepAmount )
		{
			m_StepValue = inStepAmount;
		}

		// This function is used to silently update the zoom value without calling any callbacks
		public void SilentSetZoom( float inValue )
		{
			// Clamp the value
			float clampedValue = Math.Max( m_MinZoom, Math.Min( m_MaxZoom, inValue ) );
			m_ZoomValue = clampedValue;

			UpdateUI();
		}

		// This function resets the zoom to whatever default value was set, and triggers events 
		public void ResetZoom()
		{
			ZoomValue = m_ResetValue;
		}


		private void UpdateUI()
		{
			// Turn the zoom value into a percentage to display on the label
			// If we are at the  min or max, disable the corresponding button
			int percentValue = (int) Math.Round( m_ZoomValue * 100.0f );

			percentLabel.Text		= percentValue.ToString() + "%";
			zoomInButton.Enabled	= ( m_ZoomValue < m_MaxZoom );
			zoomOutButton.Enabled	= ( m_ZoomValue > m_MinZoom );
		}

		private void zoomOutButton_Click( object sender, EventArgs e )
		{
			DecrementZoom();
		}

		private void zoomInButton_Click( object sender, EventArgs e )
		{
			IncrementZoom();
		}

		private void percentLabel_DoubleClick( object sender, EventArgs e )
		{
			ResetZoom();
		}

		private void ZoomControl_EnabledChanged( object sender, EventArgs e )
		{
			// TODO: Check if we actually still need this?
			// Ensure the buttons get disabled
			zoomInButton.Enabled	= this.Enabled;
			zoomOutButton.Enabled	= this.Enabled;
			percentLabel.Enabled	= this.Enabled;
		}
	}
}
