-- Symbol name
names =
{
	en = "Diode",
	cs = "Dioda"
}

-- Render area in base units (X1, Y1, X2, Y2)
area = {-2, -1, 2, 1}

-- Terminals
terminals = {{-2, 0}, {2, 0}}

-- Rendering
render = function (cr)
	-- The triangle
	cr.move_to (-1, -1)
	cr.line_to (1, 0)
	cr.line_to (-1, 1)
	cr.line_to (-1, -1)
	cr.stroke ()

	-- The vertical line
	cr.move_to (1, 1)
	cr.line_to (1, -1)
	cr.stroke ()

	-- The contacts
	cr.move_to (-2, 0)
	cr.line_to (-1, 0)
	cr.stroke ()

	cr.move_to (1, 0)
	cr.line_to (2, 0)
	cr.stroke ()
end

-- Register the symbol
logdiag.register ("Diode", names, area, terminals, render)


