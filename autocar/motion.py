car = Car()
car.left(speed=0.3)
car.stop()
import time
car.left(0.3)
time.sleep(0.5)
car.stop()
car.set_motors(0.3, 0.6)
time.sleep(1.0)
car.stop()
car.left_motor.value = 0.3
car.right_motor.value = 0.6
time.sleep(1.0)
car.left_motor.value = 0.0
car.right_motor.value = 0.0
import ipywidgets.widgets as widgets
from IPython.display import display

# create two sliders with range [-1.0, 1.0]
left_slider = widgets.FloatSlider(description='left', min=-1.0, max=1.0, step=0.01, orientation='vertical')
right_slider = widgets.FloatSlider(description='right', min=-1.0, max=1.0, step=0.01, orientation='vertical')

# create a horizontal box container to place the sliders next to eachother
slider_container = widgets.HBox([left_slider, right_slider])

# display the container in this cell's output
display(slider_container)
import traitlets

left_link = traitlets.link((left_slider, 'value'), (car.left_motor, 'value'))
right_link = traitlets.link((right_slider, 'value'), (car.right_motor, 'value'))
car.forward(0.3)
time.sleep(1.0)
car.stop()
left_link.unlink()
right_link.unlink()
left_link = traitlets.dlink((car.left_motor, 'value'), (left_slider, 'value'))
right_link = traitlets.dlink((car.right_motor, 'value'), (right_slider, 'value'))
# create buttons
button_layout = widgets.Layout(width='100px', height='80px', align_self='center')
stop_button = widgets.Button(description='stop', button_style='danger', layout=button_layout)
forward_button = widgets.Button(description='forward', layout=button_layout)
backward_button = widgets.Button(description='backward', layout=button_layout)
left_button = widgets.Button(description='left', layout=button_layout)
right_button = widgets.Button(description='right', layout=button_layout)

# display buttons
middle_box = widgets.HBox([left_button, stop_button, right_button], layout=widgets.Layout(align_self='center'))
controls_box = widgets.VBox([forward_button, middle_box, backward_button])
display(controls_box)
def stop(change):
    car.stop()

def step_forward(change):
    car.forward(0.4)
    time.sleep(0.5)
    car.stop()

def step_backward(change):
    car.backward(0.4)
    time.sleep(0.5)
    car.stop()

def step_left(change):
    car.left(0.3)
    time.sleep(0.5)
    car.stop()

def step_right(change):
    car.right(0.3)
    time.sleep(0.5)
    car.stop()

# link buttons to actions
stop_button.on_click(stop)
forward_button.on_click(step_forward)
backward_button.on_click(step_backward)
left_button.on_click(step_left)
right_button.on_click(step_right)
from jetbot import Heartbeat

heartbeat = Heartbeat()

# this function will be called when heartbeat 'alive' status changes
def handle_heartbeat_status(change):
    if change['new'] == Heartbeat.Status.dead:
        car.stop()

heartbeat.observe(handle_heartbeat_status, names='status')

period_slider = widgets.FloatSlider(description='period', min=0.001, max=0.5, step=0.01, value=0.5)
traitlets.dlink((period_slider, 'value'), (heartbeat, 'period'))

display(period_slider, heartbeat.pulseout)
car.left(0.2)