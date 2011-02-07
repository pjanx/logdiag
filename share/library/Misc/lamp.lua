-- Symbol name
names =
{
	en = "Lamp",
	cs = "Světelný zdroj"
}

-- Render area in base units (X1, Y1, X2, Y2)
area = {-2, -1, 2, 1}

-- Terminals
terminals = {{-2, 0}, {2, 0}}

-- Rendering
render = function (cr)
	-- The circle
	cr.save ()

	cr.arc (0, 0, 1, 0, 2 * math.pi)
	cr.stroke_preserve ()
	cr.clip ()

	cr.move_to (-1, -1)
	cr.line_to (1, 1)

	cr.move_to (1, -1)
	cr.line_to (-1, 1)
	cr.stroke ()

	cr.restore ()

	-- The contacts
	cr.move_to (-2, 0)
	cr.line_to (-1, 0)

	cr.move_to (1, 0)
	cr.line_to (2, 0)
	cr.stroke ()
end

-- Register the symbol
logdiag.register ("Lamp", names, area, terminals, render)


