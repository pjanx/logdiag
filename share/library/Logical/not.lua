-- Symbol name
names =
{
	en = "NOT",
	cs = "NOT"
}

-- Render area in base units (X1, Y1, X2, Y2)
area = {-4, -2, 4, 2}

-- Terminals
terminals = {{-4, 0}, {4, 0}}

-- Rendering
render = function (cr)
	-- The triangle
	cr.move_to (-2, -2)
	cr.line_to (2, 0)
	cr.line_to (-2, 2)
	cr.close_path ()
	cr.stroke ()

	-- The circle
	cr.new_sub_path ()
	cr.arc (2.25, 0, 0.25, 0, 2 * math.pi)
	cr.stroke ()

	-- The contacts
	cr.move_to (-4, 0)
	cr.line_to (-2, 0)
	cr.stroke ()

	cr.move_to (2.5, 0)
	cr.line_to (4, 0)
	cr.stroke ()
end

-- Register the symbol
logdiag.register ("NOT", names, area, terminals, render)


