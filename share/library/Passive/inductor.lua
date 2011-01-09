-- Symbol name
names =
{
	en = "Inductor",
	cs = "Cívka"
}

-- Render area in base units (X1, Y1, X2, Y2)
area = {-3, -1, 3, 0}

-- Terminals
terminals = {{-3, 0}, {3, 0}}

-- Rendering
render = function (cr)
	-- The left contact
	cr.move_to (-3, 0)
	cr.line_to (-2, 0)

	-- The arcs
	cr.arc (-1.5, 0, 0.5, math.pi, 0)
	cr.arc (-0.5, 0, 0.5, math.pi, 0)
	cr.arc (0.5, 0, 0.5, math.pi, 0)
	cr.arc (1.5, 0, 0.5, math.pi, 0)

	-- The right contact
	cr.line_to (3, 0)
	cr.stroke ()
end

-- Register the symbol
logdiag.register ("Inductor", names, area, terminals, render)


