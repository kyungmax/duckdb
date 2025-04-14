/*****************************************************************************
 *
 *		QUERY :
 *				CREATE [ OR REPLACE ] MATERIALIZED VIEW <matviewname> AS SelectStmt
 *
 * Note: Now, only for Iceberg tables
 *****************************************************************************/
CreateMatViewStmt:
		CREATE_P OptTemp MATERIALIZED VIEW qualified_name AS SelectStmt
				{
					PGCreateMatViewStmt *n = makeNode(PGCreateMatViewStmt);
                    n->view = $5;
                    n->view->relpersistence = $2;
                    n->query = $7;
                    n->onconflict = PG_ERROR_ON_CONFLICT;
                    $$ = (PGNode *) n;
				}
		| CREATE_P OR REPLACE OptTemp MATERIALIZED VIEW qualified_name AS SelectStmt
				{
					PGCreateMatViewStmt *n = makeNode(PGCreateMatViewStmt);
                    n->view = $7;
                    n->view->relpersistence = $4;
                    n->query = $9;
                    n->onconflict = PG_ERROR_ON_CONFLICT;
                    $$ = (PGNode *) n;
				}
		;
