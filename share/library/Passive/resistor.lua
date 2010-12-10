-- Symbol name
names =
{
	en = "Resistor",
	cs = "Rezistor"
}

-- Render area in base units (X1, Y1, X2, Y2)
area = {-2, -0.5, 2, 0.5}

-- Terminals
terminals = {{-2, 0}, {2, 0}}

-- Rendering
render = function (cr)
	-- The rectangle
	cr.move_to (-1.5, -0.5)
	cr.line_to (1.5, -0.5)
	cr.line_to (1.5, 0.5)
	cr.line_to (-1.5, 0.5)
	cr.line_to (-1.5, -0.5)
	cr.stroke ()

	-- The contacts
	cr.move_to (-2, 0)
	cr.line_to (-1.5, 0)
	cr.stroke ()

	cr.move_to (1.5, 0)
	cr.line_to (2, 0)
	cr.stroke ()
end

-- Register the symbol
logdiag.register ("Resistor", names, area, terminals, render)


