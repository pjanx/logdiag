-- Symbol name
names =
{
	en = "Capacitor",
	cs = "Kondenzátor"
}

-- Render area in base units (X1, Y1, X2, Y2)
area = {-2, -1, 2, 1}

-- Terminals
terminals = {{-2, 0}, {2, 0}}

-- Rendering
render = function (cr)
	-- The vertical lines
	cr.move_to (-0.2, -1)
	cr.line_to (-0.2, 1)
	cr.stroke ()

	cr.move_to (0.2, -1)
	cr.line_to (0.2, 1)
	cr.stroke ()

	-- The contacts
	cr.move_to (-2, 0)
	cr.line_to (-0.2, 0)
	cr.stroke ()

	cr.move_to (0.2, 0)
	cr.line_to (2, 0)
	cr.stroke ()
end

-- Register the symbol
logdiag.register ("Capacitor", names, area, terminals, render)


